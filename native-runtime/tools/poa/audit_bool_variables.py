"""Count original BoolVariable and LetBool forms without executing scripts."""
import argparse
import collections
import json
import pathlib
from inspect_packages import inspect
from object_payloads import function_header
from bytecode import Decoder


def main():
    p = argparse.ArgumentParser()
    p.add_argument('game'); p.add_argument('output')
    a = p.parse_args()
    wrappers, targets = collections.Counter(), collections.Counter()
    functions = 0
    for path in sorted(pathlib.Path(a.game).rglob('*')):
        if path.suffix.lower() != '.u':
            continue
        model = inspect(path); data = path.read_bytes()
        for export in model['exports']:
            if export['class'] != 'Function':
                continue
            reader, header = function_header(data[export['offset']:export['offset'] + export['size']], model['names'])
            decoder = Decoder(reader, model['names'], len(model['imports']), len(model['exports']), header['declaredBytecodeMemorySize'])
            decoder.decode(); functions += 1
            for index, token in enumerate(decoder.tokens):
                if token['opcode'] not in (0x2d, 0x14):
                    continue
                if index + 1 >= len(decoder.tokens):
                    raise ValueError('Missing child of Boolean expression')
                child = decoder.tokens[index + 1]
                if child['depth'] != token['depth'] + 1:
                    raise ValueError('Invalid Boolean expression child depth')
                (wrappers if token['opcode'] == 0x2d else targets)[f"0x{child['opcode']:02x}"] += 1
    result = dict(decodedFunctions=functions, wrapperChildren=dict(wrappers),
                  letBoolTargets=dict(targets), supportedWrapperChildren=['0x00', '0x01', '0x02'],
                  scope='Original bytecode structure only; packed native Boolean layouts and gameplay unverified')
    pathlib.Path(a.output).write_text(json.dumps(result, indent=2))
    print(result)


if __name__ == '__main__':
    main()
