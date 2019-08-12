#pragma once

#include <ostream>
#include <istream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <initializer_list>
#include <utility>
#include <stdint.h>

namespace Swan {

struct SRFVal {
	virtual ~SRFVal() = default;

	virtual void serialize(std::ostream &os) = 0;
	virtual void parse(std::istream &is) = 0;

	static SRFVal *read(std::istream &is);
};

struct SRFObject: SRFVal {
	SRFObject() {}
	SRFObject(std::initializer_list<std::pair<std::string, SRFVal *>> &lst);
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::unordered_map<std::string, std::unique_ptr<SRFVal>> val;
};

struct SRFArray: SRFVal {
	SRFArray() {}
	SRFArray(std::initializer_list<SRFVal *> &lst);
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<std::unique_ptr<SRFVal>> val;
};

struct SRFString: SRFVal {
	SRFString(): val("") {}
	SRFString(const std::string &v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::string val;
};

struct SRFInt: SRFVal {
	SRFInt(): val(0) {}
	SRFInt(int32_t v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	int32_t val;
};

struct SRFFloat: SRFVal {
	SRFFloat(): val(0) {}
	SRFFloat(float v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	float val;
};

struct SRFDouble: SRFVal {
	SRFDouble(): val(0) {}
	SRFDouble(double v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	double val;
};

struct SRFNone: SRFVal {
	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;
};

struct SRFByteArray: SRFVal {
	SRFByteArray() = default;
	SRFByteArray(std::vector<uint8_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<uint8_t> val;
};

struct SRFWordArray: SRFVal {
	SRFWordArray() = default;
	SRFWordArray(std::vector<uint16_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<uint16_t> val;
};

struct SRFIntArray: SRFVal {
	SRFIntArray() = default;
	SRFIntArray(std::vector<int32_t> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<int32_t> val;
};

struct SRFFloatArray: SRFVal {
	SRFFloatArray() = default;
	SRFFloatArray(std::vector<float> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<float> val;
};

struct SRFDoubleArray: SRFVal {
	SRFDoubleArray() = default;
	SRFDoubleArray(std::vector<double> v): val(v) {}

	void serialize(std::ostream &os) override;
	void parse(std::istream &os) override;

	std::vector<double> val;
};

}
