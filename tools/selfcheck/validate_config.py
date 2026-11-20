#!/usr/bin/env python3
"""Validate the repository schema subset without third-party dependencies."""
import argparse
import json
from pathlib import Path
import re

SCHEMA = Path(__file__).resolve().parents[2] / 'config/schemas/capture.schema.json'


def validate(value, schema, path='$', allow_incomplete=False):
    errors = []
    kinds = {'object': dict, 'array': list, 'string': str, 'integer': int, 'null': type(None)}
    expected = schema['type']
    expected = expected if isinstance(expected, list) else [expected]
    if not any(type(value) is kinds[t] for t in expected):
        return [f'{path}: expected {expected}']
    if value is None:
        return [] if allow_incomplete else [f'{path}: unresolved value is not allowed']
    if 'const' in schema and value != schema['const']:
        errors.append(f'{path}: expected {schema["const"]!r}')
    if 'enum' in schema and value not in schema['enum']:
        errors.append(f'{path}: unsupported value {value!r}')
    if isinstance(value, dict):
        for key in schema.get('required', []):
            if key not in value:
                errors.append(f'{path}.{key}: required')
        for key, item in value.items():
            if key not in schema['properties']:
                errors.append(f'{path}.{key}: unknown field')
            else:
                errors.extend(validate(item, schema['properties'][key], f'{path}.{key}', allow_incomplete))
    elif isinstance(value, list):
        if len(value) < schema.get('minItems', 0) or len(value) > schema.get('maxItems', float('inf')):
            errors.append(f'{path}: invalid item count')
        if schema.get('uniqueItems') and len({json.dumps(x, sort_keys=True) for x in value}) != len(value):
            errors.append(f'{path}: duplicate items')
        for index, item in enumerate(value):
            errors.extend(validate(item, schema['items'], f'{path}[{index}]', allow_incomplete))
    elif isinstance(value, str):
        if len(value) < schema.get('minLength', 0):
            errors.append(f'{path}: empty value')
        if 'pattern' in schema and not re.search(schema['pattern'], value):
            errors.append(f'{path}: invalid path')
    elif type(value) is int and value < schema.get('minimum', value):
        errors.append(f'{path}: below minimum')
    return errors


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f'duplicate JSON key: {key}')
        result[key] = value
    return result


def load_json(path):
    def reject_constant(value):
        raise ValueError(f'invalid JSON constant: {value}')
    return json.loads(Path(path).read_text(), object_pairs_hook=unique_object, parse_constant=reject_constant)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('config', type=Path)
    parser.add_argument('--allow-incomplete', action='store_true', help='design examples only')
    args = parser.parse_args()
    try:
        errors = validate(load_json(args.config), load_json(SCHEMA), allow_incomplete=args.allow_incomplete)
    except (OSError, ValueError) as exc:
        parser.exit(2, f'{exc}\n')
    if errors:
        parser.exit(1, '\n'.join(errors) + '\n')
    print('Configuration structure valid' + (' (incomplete design values permitted)' if args.allow_incomplete else '; hardware capability checks still required'))


if __name__ == '__main__':
    main()
