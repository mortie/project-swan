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

struct SRF {
	virtual ~SRF() = default;

	virtual void serialize(std::ostream &os) const = 0;
	virtual void parse(std::istream &is) = 0;
	virtual std::ostream &pretty(std::ostream &os) const = 0;

	static SRF *read(std::istream &is);
};

struct SRFObject: SRF {
	SRFObject() {}
	SRFObject(std::initializer_list<std::pair<std::string, SRF *>> &lst);

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::unordered_map<std::string, std::unique_ptr<SRF>> val;
};

struct SRFArray: SRF {
	SRFArray() {}
	SRFArray(std::initializer_list<SRF *> &lst);

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<std::unique_ptr<SRF>> val;
};

struct SRFString: SRF {
	SRFString(): val("") {}
	SRFString(const std::string &v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::string val;
};

struct SRFInt: SRF {
	SRFInt(): val(0) {}
	SRFInt(int32_t v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	int32_t val;
};

struct SRFFloat: SRF {
	SRFFloat(): val(0) {}
	SRFFloat(float v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	float val;
};

struct SRFDouble: SRF {
	SRFDouble(): val(0) {}
	SRFDouble(double v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	double val;
};

struct SRFNone: SRF {
	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;
};

struct SRFByteArray: SRF {
	SRFByteArray() = default;
	SRFByteArray(std::initializer_list<uint8_t> lst): val(lst) {}
	SRFByteArray(std::vector<uint8_t> v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<uint8_t> val;
};

struct SRFWordArray: SRF {
	SRFWordArray() = default;
	SRFWordArray(std::initializer_list<uint16_t> v): val(v) {}
	SRFWordArray(std::vector<uint16_t> v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<uint16_t> val;
};

struct SRFIntArray: SRF {
	SRFIntArray() = default;
	SRFIntArray(std::initializer_list<int> v): val(v) {}
	SRFIntArray(std::vector<int32_t> v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<int32_t> val;
};

struct SRFFloatArray: SRF {
	SRFFloatArray() = default;
	SRFFloatArray(std::initializer_list<float> v): val(v) {}
	SRFFloatArray(std::vector<float> v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<float> val;
};

struct SRFDoubleArray: SRF {
	SRFDoubleArray() = default;
	SRFDoubleArray(std::initializer_list<double> v): val(v) {}
	SRFDoubleArray(std::vector<double> v): val(v) {}

	void serialize(std::ostream &os) const override;
	void parse(std::istream &os) override;
	std::ostream &pretty(std::ostream &os) const override;

	std::vector<double> val;
};

}
