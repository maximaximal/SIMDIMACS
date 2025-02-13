import sys
from writer import CPPWriter as Writer

SIMD_ELEMENT_SIZES = [1, 2, 4, 8, 16]

class DigitsSpan(object):
    # range: [first, last] - include the both ends
    def __init__(self, first, last):
        assert first <= last

        self.first = first
        self.last  = last

        assert self.digits() <= 16


    def digits(self):
        return self.last - self.first + 1


    def simd_size(self):
        d = self.digits()
        if d == 1:
            return 1
        if d == 2:
            return 2
        if d <= 4:
            return 4
        if d <= 8:
            return 8
        return 16;

    def __str__(self):
        return "<%d,%d>" % (self.first, self.last)

    __repr__ = __str__

class OperatorSpan(object):
    def __init__(self, first, last):
        assert first <= last

        self.first = first
        self.last = last

    def digits(self):
        return self.last - self.first + 1

DIGIT = 'd'
SPACE = '_'

class Parser(object):
    def __init__(self, number):
        assert number >= 0
        assert number < 65536

        self.number = number
        self.image = self.__convert_to_string(number)


    def get_spans(self):
        prev  = SPACE
        prev_prev = SPACE
        start = None
        op_start = 0
        spans = []
        for i, c in enumerate(self.image):
            if c == prev:
                if prev == prev_prev and c == SPACE and (i - op_start) >= 1:
                    spans.append(OperatorSpan(op_start, i - 1))
                    # Print all positions where this happens. The last entry
                    # shall always be the op:
                    #
                    # print(self.image[0:i])
                    #
                    # With the first operator span, we want to exit this block!
                    return (spans, [])
                continue

            if c == DIGIT: # transition
                start = i
            else:
                # Note: a digits span which not ends within the chunk
                #       doesn't appear in the result (we don't know how to parse it)
                spans.append(DigitsSpan(start, i - 1))
                start = None
                op_start = i + 1

            prev = c

        if start is not None:
            incomplete = [DigitsSpan(start, 15)]
        else:
            incomplete = []

        return (spans, incomplete)


    def __convert_to_string(self, x):
        s = ''
        for i in range(16):
            if x & (1 << i):
                s += DIGIT
            else:
                s += SPACE

        return s


class Optimizer(object):
    def __init__(self, spans):
        self.spans = spans


    def get_best(self):
        best = None
        best_size = None
        for element_size in SIMD_ELEMENT_SIZES:

            res = self.__pack(element_size)
            if res is None:
                continue

            if len(res) > 0 and isinstance(res[len(res) - 1], OperatorSpan):
                return (element_size, res)

            if best is None or len(res) > len(best):
                best = res;
                best_size = element_size

        if best is not None:
            return (best_size, best)
        else:
            return None


    def __pack(self, element_size, vector_size = 16):
        max_size = vector_size / element_size
        result = []
        for r in self.spans:
            if r.digits() <= element_size:
                result.append(r)
                if len(result) == max_size or isinstance(r, OperatorSpan):
                    break
            else:
                break

        if len(result) > 0:
            return result
        else:
            return None


class BlockInfo(object):

    __slots__ = ("id", "image", "first_skip", "total_skip", "trailing_op",
                 "spans", "all_spans", "element_size", "shuffle_digits",
                 "shuffle_signs")

    def __init__(self, number):
        self.id              = number
        self.first_skip      = 0
        self.total_skip      = 0
        self.trailing_op     = False
        self.spans           = []
        self.all_spans       = []
        self.element_size    = 0
        self.shuffle_digits  = []
        self.shuffle_signs   = []

    def build_pshubf_masks(self):
        self.build_shuffle_digit()
        self.build_shuffle_signs()

    def build_shuffle_digit(self):
        self.shuffle_digits = [0x80] * 16
        for element, r in enumerate(self.spans):
            index  = element * self.element_size
            index += self.element_size - r.digits() # align to "right" within the vector's element

            for i in range(r.first, r.last + 1):
                self.shuffle_digits[index] = i
                index += 1

    def build_shuffle_signs(self):
        self.shuffle_signs = [0x80] * 16
        for element, r in enumerate(self.spans):
            index = element * self.element_size
            for i in range(self.element_size):
                self.shuffle_signs[index + i] = r.first

    def get_invalid_sign_mask(self):
        result = 0
        for r in self.all_spans:
            if isinstance(r, OperatorSpan):
                # We don't care for what comes after the operator, so everything
                # following the last digit is allowed, as this is handled
                # afterwards / later.
                for i in range(r.last, 16):
                    bit = 1 << i
                    result |= bit
                break

            if r.digits() <= 1:
                continue

            # only first character of span might be '+' or '-'
            bit = 1 << r.first
            result |= bit

        if self.all_spans:
            # if last span has just one char it might also be a sign
            last = self.all_spans[-1]
            if not isinstance(last, OperatorSpan) and last.digits() == 1:
                bit = 1 << 15
                result |= bit

        # negate result, to avoid negation in runtime
        return ~result & 0xffff

    def __str__(self):
        param = (
            self.id,
            self.first_skip,
            self.total_skip,
            self.element_size,
            self.spans
        )

        return "<BlockInfo#%04x {first_skip=%d, total_skip=%d, " \
               "element_size=%d, spans=%s}>" % param


class Generator(object):
    def run(self):
        for i in range(2**16):
            yield self.__get_structure(i)


    def __get_structure(self, number):
        parser = Parser(number)
        spans, incomplete_span = parser.get_spans()

        opt = Optimizer(spans)
        ret = opt.get_best()

        block = BlockInfo(number)
        block.trailing_op = isinstance(spans[len(spans) - 1], OperatorSpan) if len(spans) > 0 else False
        block.image = parser.image
        block.all_spans = spans + incomplete_span
        if ret is not None:
            element_size, items = ret

            block.first_skip   = items[0].first
            block.spans        = items
            block.element_size = element_size

            block.total_skip = items[-1].last + 1
            try:
                image = parser.image
                while image[block.total_skip] == '_':
                    block.total_skip += 1
            except IndexError:
                pass
        else:
            if number != 0: # there are digits at the end of chunk
                image = parser.image
                block.first_skip = image.index(DIGIT)

        block.build_pshubf_masks()

        return block


def main(path):
    gen  = Generator()
    data = list(gen.run())

    writer = Writer(data)
    writer.save(path)

if __name__ == '__main__':
    if len(sys.argv) < 1:
        print("Usage: script output-path")
        sys.exit(1)

    main(sys.argv[1])
