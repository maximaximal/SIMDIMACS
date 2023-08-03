ITEM_PATTERN = \
    "/* %(ID)04x %(SCALAR_COST)5.2f %(SSE_COST)5.2f */ {" \
    "%(FIRST_SKIP)s," \
    "%(TOTAL_SKIP)s," \
    "%(ELEMENT_COUNT)s," \
    "%(TRAILING_OP)s," \
    "%(CONVERSION)s," \
    "0x%(INVALID_SIGN_MASK)04x," \
    "{%(SHUFFLE_DIGITS)s}," \
    "{%(SHUFFLE_SIGNS)s}" \
    "}"

FILE_PATTERN = """
#pragma once

#include "simdimacs-internal.h"

simdimacs_blockinfo simdimacs_blocks[%(COUNT)d] = {
%(ITEMS)s
};
"""

from cost import scalar_cost, SSE_cost

class CPPWriter(object):
    def __init__(self, data):
        self.data = data
        pass

    def save(self, path):
        tmp = [self._render_item(item) for item in self.data]
        params = {
            'COUNT': len(tmp),
            'ITEMS': ',\n'.join(tmp),
        }

        text = FILE_PATTERN % params

        with open(path, 'w') as f:
            f.write(text)


    def _render_item(self, block):
        # If there is some op, the last element is not an element to be read as
        # number!
        elem_count = len(block.spans)
        if block.trailing_op:
            elem_count -= 1

        params = {
            'ID'                : block.id,
            'FIRST_SKIP'        : block.first_skip,
            'TOTAL_SKIP'        : block.total_skip,
            'ELEMENT_COUNT'     : elem_count,
            'TRAILING_OP'       : 1 if block.trailing_op else 0,
            'CONVERSION'        : self.get_conversion_enum(block),
            'INVALID_SIGN_MASK' : block.get_invalid_sign_mask(),
            'SHUFFLE_DIGITS'    : self._make_c_array(block.shuffle_digits),
            'SHUFFLE_SIGNS'     : self._make_c_array(block.shuffle_signs),
            'SCALAR_COST'       : scalar_cost(block).value(),
            'SSE_COST'          : SSE_cost(block).value(),
        }

        return ITEM_PATTERN % params


    def _make_c_array(self, numbers):
        return ','.join('0x%02x' % x for x in numbers)

    def get_conversion_enum(self, block):
        if block.element_size == 0:
            return 'Empty'

        if block.element_size == 1:
            return 'SSE1Digit'

        if block.element_size == 2:
            return 'SSE2Digits'

        if block.element_size == 4:
            if all(r.digits() == 3 for r in block.spans):
                return 'SSE3Digits'
            else:
                return 'SSE4Digits'

        if block.element_size == 8:
            return 'SSE8Digits'

        if block.element_size == 16:
            return 'Scalar'

        assert False
