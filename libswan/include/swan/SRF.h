#pragma once

#include <ostream>
#include <istream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <stdint.h>

namespace Swan {

struct SRFTag {
	virtual ~SRFTag() = default;

	virtual void serialize(std::ostream &os) = 0;
	virtual void parse(std::istream &is) = 0;

	static SRFTag *read(std::istream &is);
};

struct SRFObject: SRFTag {
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::unordered_map<std::string, std::unique_ptr<SRFTag>> val;
};

struct SRFArray: SRFTag {
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<std::unique_ptr<SRFTag>> val;
};

struct SRFString: SRFTag {
	SRFString(): val("") {}
	SRFString(const std::string &v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::string val;
};

struct SRFInt: SRFTag {
	SRFInt(): val(0) {}
	SRFInt(int32_t v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	int32_t val;
};

struct SRFFloat: SRFTag {
	SRFFloat(): val(0) {}
	SRFFloat(float v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	float val;
};

struct SRFDouble: SRFTag {
	SRFDouble(): val(0) {}
	SRFDouble(double v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	double val;
};

struct SRFNone: SRFTag {
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;
};

struct SRFByteArray: SRFTag {
	SRFByteArray() = default;
	SRFByteArray(std::vector<uint8_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<uint8_t> val;
};

struct SRFWordArray: SRFTag {
	SRFWordArray() = default;
	SRFWordArray(std::vector<uint16_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<uint16_t> val;
};

struct SRFIntArray: SRFTag {
	SRFIntArray() = default;
	SRFIntArray(std::vector<int32_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<int32_t> val;
};

struct SRFFloatArray: SRFTag {
	SRFFloatArray() = default;
	SRFFloatArray(std::vector<float> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<float> val;
};

struct SRFDoubleArray: SRFTag {
	SRFDoubleArray() = default;
	SRFDoubleArray(std::vector<double> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<double> val;
};

}
