#include "rle.h"

#include "lib/test.h"

#include <iostream>
#include <span>
#include <vector>
#include <cstring>

using namespace Swan;

TEST("Round-trip RLE8")
{
	unsigned char input[] = {
		'a', 'b', 'b', 'c',
		'b', 'b', 'b', 'b',
		'z', 'z', 'z', 'x'
	};
	std::vector<uint8_t> output;
	rleEncode8(output, std::span(input));

	unsigned char decoded_output[12];
	expecteq(rleDecode8(decoded_output, output), 12);
	expect(memcmp(input, decoded_output, 12) == 0);

	// Even a too small output buffer should return the full size,
	// and the first few bytes should be correct
	unsigned char too_small_output[6];
	expecteq(rleDecode8(too_small_output, output), 12);
	expect(memcmp(input, too_small_output, 6) == 0);
}

TEST("Round-trip RLE16SRO")
{
	uint16_t input[] = {
		650, 650, 650, 650,
		12, 12, 973, 12,
		1234, 1235, 1233, 1233,
	};
	std::vector<uint8_t> output;
	rleEncode16SRO(output, std::span(input));

	uint16_t decoded_output[12];
	expecteq(rleDecode16SRO(decoded_output, output), 12);
	expect(memcmp(input, decoded_output, 12) == 0);

	// Even a too small output buffer should return the full size,
	// and the first few bytes should be correct
	uint16_t too_small_output[6];
	expecteq(rleDecode16SRO(too_small_output, output), 12);
	expect(memcmp(input, too_small_output, 6) == 0);
}
