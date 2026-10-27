"""Reads DayZ configs straight from PBO files.

read_pbo_file(pbo, name)  -> bytes of one file inside a PBO (no full unpack)
parse_rap(data)           -> binarized config.bin as nested dicts ('__parent' holds the base class)
load_classes(pbos, root)  -> every class under one root (e.g. 'CfgVehicles') from many PBOs,
                             merged in load order the way the game does
"""
import csv, io, os, re, struct


def _pbo_entries(f):
    """Yield (name, size, data_offset) for each file in an open PBO."""
    def cstr():
        out = bytearray()
        while (c := f.read(1)) not in (b'\0', b''):
            out += c
        return out.decode('utf-8', 'replace')

    entries = []
    while True:
        name = cstr()
        mime, orig, _res, _ts, size = struct.unpack('<5I', f.read(20))
        if name == '' and mime == 0x56657273:  # 'Vers' product entry
            while cstr() != '':
                cstr()
            continue
        if name == '':
            break
        entries.append((name, size, orig))
    offset = f.tell()
    for name, size, orig in entries:
        yield name, size, offset, orig
        offset += size


def read_pbo_file(pbo, wanted):
    """Return the bytes of the file called `wanted` (case-insensitive) inside `pbo`, or None."""
    wanted = wanted.lower()
    with open(pbo, 'rb') as f:
        for name, size, offset, orig in list(_pbo_entries(f)):
            if name.lower() == wanted:
                if orig and orig != size:
                    return None  # compressed entry, not supported
                f.seek(offset)
                return f.read(size)
    return None


def list_pbo_files(pbo):
    with open(pbo, 'rb') as f:
        return [name for name, *_ in _pbo_entries(f)]


def _cstr(b, i):
    j = b.index(b'\0', i)
    return b[i:j].decode('latin1'), j + 1


def _cint(b, i):
    v = s = 0
    while True:
        c = b[i]; i += 1
        v |= (c & 0x7f) << s; s += 7
        if not c & 0x80:
            return v, i


def _array(b, i):
    n, i = _cint(b, i)
    out = []
    for _ in range(n):
        t = b[i]; i += 1
        if t == 0:
            s, i = _cstr(b, i); out.append(s)
        elif t == 1:
            out.append(struct.unpack_from('<f', b, i)[0]); i += 4
        elif t == 2:
            out.append(struct.unpack_from('<i', b, i)[0]); i += 4
        elif t == 3:
            a, i = _array(b, i); out.append(a)
        elif t == 4:  # variable reference
            s, i = _cstr(b, i); out.append(s)
        else:
            raise ValueError(f'array entry type {t}')
    return out, i


def _class(b, off):
    i = off
    parent, i = _cstr(b, i)
    n, i = _cint(b, i)
    body = {'__parent': parent}
    for _ in range(n):
        t = b[i]; i += 1
        if t == 0:
            name, i = _cstr(b, i)
            o = struct.unpack_from('<I', b, i)[0]; i += 4
            body[name] = _class(b, o)
        elif t == 1:
            sub = b[i]; i += 1
            name, i = _cstr(b, i)
            if sub == 0:
                v, i = _cstr(b, i)
            elif sub == 1:
                v = struct.unpack_from('<f', b, i)[0]; i += 4
            else:
                v = struct.unpack_from('<i', b, i)[0]; i += 4
            body[name] = v
        elif t in (2, 5):
            if t == 5:
                i += 4  # flags of an "+=" array
            name, i = _cstr(b, i)
            v, i = _array(b, i)
            body[name] = v
        elif t in (3, 4):  # external declaration / delete
            name, i = _cstr(b, i)
    return body


def parse_rap(data):
    if data[:4] != b'\0raP':
        raise ValueError('not a binarized config')
    return _class(data, 16)


def _cpp_tokens(text):
    """Tokens of a text config.cpp, with comments and preprocessor lines removed."""
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    text = re.sub(r'^\s*#.*$', ' ', text, flags=re.M)
    return re.findall(r'"(?:[^"]|"")*"|\+=|[{}\[\];:=,]|[^\s{}\[\];:=,"]+', text)


def _cpp_value(tok):
    if tok.startswith('"'):
        return tok[1:-1].replace('""', '"')
    try:
        return int(tok)
    except ValueError:
        try:
            return float(tok)
        except ValueError:
            return tok


def _cpp_array(toks, i):
    """Parse {a, b, {c}} starting at '{'. Returns (list, index after '}')."""
    out, i = [], i + 1
    while toks[i] != '}':
        if toks[i] == '{':
            sub, i = _cpp_array(toks, i)
            out.append(sub)
        elif toks[i] != ',':
            out.append(_cpp_value(toks[i])); i += 1
        else:
            i += 1
    return out, i + 1


def _cpp_body(toks, i, parent=''):
    body = {'__parent': parent}
    while i < len(toks) and toks[i] != '}':
        t = toks[i]
        if t == 'class':
            name = toks[i + 1]; i += 2
            base = ''
            if toks[i] == ':':
                base = toks[i + 1]; i += 2
            if toks[i] == '{':
                body[name], i = _cpp_body(toks, i + 1, base)
                i += 1  # '}'
            if i < len(toks) and toks[i] == ';':
                i += 1
        elif t == 'delete':
            i += 3
        elif t == ';':
            i += 1
        else:
            name = t; i += 1
            if toks[i] == '[':
                i += 2  # '[' ']'
                i += 1  # '=' or '+='
                body[name], i = _cpp_array(toks, i)
            else:
                i += 1  # '='
                vals = []
                while toks[i] != ';':
                    vals.append(toks[i]); i += 1
                body[name] = _cpp_value(' '.join(vals)) if len(vals) != 1 else _cpp_value(vals[0])
            if i < len(toks) and toks[i] == ';':
                i += 1
    return body, i


def parse_cpp(text):
    """Parse a text config.cpp into the same shape as parse_rap. Macros are not expanded."""
    body, _ = _cpp_body(_cpp_tokens(text), 0)
    return body


def read_configs(pbo):
    """Every config inside a PBO (config.bin or config.cpp, in any folder), parsed.

    Configs that can't be read (compressed, encrypted or using unsupported syntax) are skipped.
    """
    names = [n for n in list_pbo_files(pbo) if n.lower().rsplit('\\', 1)[-1] in ('config.bin', 'config.cpp')]
    names.sort(key=lambda n: (n.count('\\'), n.lower()))  # root config first
    out = []
    for name in names:
        data = read_pbo_file(pbo, name)
        if not data:
            continue
        try:
            if data[:4] == b'\0raP':
                out.append(parse_rap(data))
            else:
                out.append(parse_cpp(data.decode('utf-8', 'replace')))
        except (ValueError, IndexError, KeyError):
            continue
    return out


def load_classes(pbos, root='CfgVehicles'):
    """Merge `root` classes from every PBO's config.bin, later PBOs overriding earlier ones.

    Config names are case-insensitive, so classes are keyed by lower-case name.
    Returns {class_lower: {'name': str, 'parent': str_lower, 'props': dict, 'source': str}}.
    """
    classes = {}
    for pbo in pbos:
        for cfg in read_configs(pbo):
            section = next((v for k, v in cfg.items() if k.lower() == root.lower()), {})
            for name, body in section.items():
                if name.startswith('__') or not isinstance(body, dict):
                    continue
                entry = classes.setdefault(name.lower(), {'name': name, 'parent': '', 'props': {},
                                                           'source': pbo})
                if body['__parent']:
                    entry['parent'] = body['__parent'].lower()
                entry['props'].update({k: v for k, v in body.items() if not k.startswith('__')})
    return classes


def resolve(classes, name, prop, default=None):
    """Value of `prop` for a class, following parents. Returns (value, class_that_defines_it)."""
    name, seen = name.lower(), set()
    while name and name in classes and name not in seen:
        seen.add(name)
        if prop in classes[name]['props']:
            return classes[name]['props'][prop], name
        name = classes[name]['parent']
    return default, None


def ancestors(classes, name):
    """Lower-case class names from `name` up to the root."""
    out, name, seen = [], name.lower(), set()
    while name and name not in seen:
        seen.add(name); out.append(name)
        name = classes.get(name, {}).get('parent', '')
    return out


def load_stringtables(pbos):
    """English text for $STR_ keys from every stringtable.csv found in the PBOs."""
    table = {}
    for pbo in pbos:
        for name in list_pbo_files(pbo):
            if name.lower().endswith('stringtable.csv'):
                data = read_pbo_file(pbo, name)
                if not data:
                    continue
                rows = csv.reader(io.StringIO(data.decode('utf-8', 'replace')))
                header = [h.strip().lower() for h in next(rows, [])]
                col = header.index('english') if 'english' in header else (header.index('original') if 'original' in header else 1)
                for row in rows:
                    if len(row) > col and row[0]:
                        table[row[0].strip().lower()] = row[col]
    return table
