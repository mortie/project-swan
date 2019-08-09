#include "SRF.h"

namespace Swan {

enum class Type {
	OBJECT = 0,
	ARRAY = 1,
	STRING = 2,
	INT = 3,
	FLOAT = 4,
	DOUBLE = 5,
	NONE = 6,
	BYTE_ARRAY = 7,
	WORD_ARRAY = 8,
	INT_ARRAY = 9,
	FLOAT_ARRAY = 10,
	DOUBLE_ARRAY = 11,
};

static void writeByte(std::ostream &os, uint8_t val) {
	os.put(val);
}

static uint8_t readByte(std::istream &is) {
	return is.get();
}

static void writeWord(std::ostream &os, uint16_t val) {
	os.put((val & 0xff00u) >> 8);
	os.put(val & 0x00ffu);
}

static uint16_t readWord(std::istream &is) {
	return (uint16_t)(
			((uint16_t)is.get()) << 8 |
			((uint16_t)is.get()));
}

static void writeInt(std::ostream &os, int32_t val) {
	uint32_t uval = val;
	os.put((uval & 0xff000000u) << 24);
	os.put((uval & 0x00ff0000u) << 16);
	os.put((uval & 0x0000ff00u) << 8);
	os.put((uval & 0x000000ffu));
}

int32_t readInt(std::istream &is) {
	return (int32_t)(
			((uint32_t)is.get() << 24) |
			((uint32_t)is.get() << 16) |
			((uint32_t)is.get() << 8) |
			((uint32_t)is.get()));
}

static void writeFloat(std::ostream &os, float val) {
	uint32_t uval = (uint32_t)val;
	os.put((uval & 0xff000000u) << 24);
	os.put((uval & 0x00ff0000u) << 16);
	os.put((uval & 0x0000ff00u) << 8);
	os.put((uval & 0x000000ffu));
}

static float readFloat(std::istream &is) {
	return (float)(
			((uint32_t)is.get() << 24) |
			((uint32_t)is.get() << 16) |
			((uint32_t)is.get() << 8) |
			((uint32_t)is.get()));
}

static void writeDouble(std::ostream &os, double val) {
	uint64_t uval = (uint64_t)val;
	os.put((uval & 0xff00000000000000lu) << 56);
	os.put((uval & 0x00ff000000000000lu) << 48);
	os.put((uval & 0x0000ff0000000000lu) << 40);
	os.put((uval & 0x000000ff00000000lu) << 32);
	os.put((uval & 0x00000000ff000000lu) << 24);
	os.put((uval & 0x0000000000ff0000lu) << 16);
	os.put((uval & 0x000000000000ff00lu) << 8);
	os.put((uval & 0x00000000000000fflu));
}

static double readDouble(std::istream &is) {
	return (double)(
			((uint64_t)is.get() << 56) |
			((uint64_t)is.get() << 48) |
			((uint64_t)is.get() << 40) |
			((uint64_t)is.get() << 32) |
			((uint64_t)is.get() << 24) |
			((uint64_t)is.get() << 16) |
			((uint64_t)is.get() << 8) |
			((uint64_t)is.get()));
}

static void writeString(std::ostream &os, const std::string &str) {
	writeInt(os, str.size());
	os.write(str.c_str(), str.size());
}

static std::string readString(std::istream &is) {
	std::string str;
	int32_t len = readInt(is);
	str.reserve(len);
	is.read(str.data(), len);
	return str;
}

SRFTag *SRFTag::read(std::istream &is) {
	Type type = (Type)readByte(is);
	SRFTag *tag;

	switch (type) {
	case Type::OBJECT:
		tag = new SRFObject(); break;
	case Type::ARRAY:
		tag = new SRFArray(); break;
	case Type::STRING:
		tag = new SRFString(); break;
	case Type::INT:
		tag = new SRFInt(); break;
	case Type::FLOAT:
		tag = new SRFFloat(); break;
	case Type::DOUBLE:
		tag = new SRFDouble(); break;
	case Type::NONE:
		tag = new SRFNone(); break;
	case Type::BYTE_ARRAY:
		tag = new SRFByteArray(); break;
	case Type::WORD_ARRAY:
		tag = new SRFWordArray(); break;
	case Type::INT_ARRAY:
		tag = new SRFIntArray(); break;
	case Type::FLOAT_ARRAY:
		tag = new SRFFloatArray(); break;
	case Type::DOUBLE_ARRAY:
		tag = new SRFDoubleArray(); break;
	}

	tag->parse(is);
	return tag;
}

void SRFObject::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::OBJECT);
	writeInt(os, val.size());

	for (auto &pair: val) {
		writeString(os, pair.first);
		pair.second->serialize(os);
	}
}

void SRFObject::parse(std::istream &is) {
	int32_t count = readInt(is);

	for (int32_t i = 0; i < count; ++i) {
		std::string key = readString(is);
		val[key] = std::unique_ptr<SRFTag>(SRFTag::read(is));
	}
}

void SRFArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::ARRAY);
	writeInt(os, val.size());

	for (auto &tag: val) {
		tag->serialize(os);
	}
}

void SRFArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);

	for (int32_t i = 0; i < count; ++i) {
		val[i] = std::unique_ptr<SRFTag>(SRFTag::read(is));
	}
}

void SRFString::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::STRING);
	writeString(os, val);
}

void SRFString::parse(std::istream &is) {
	val = readString(is);
}

void SRFInt::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::INT);
	writeInt(os, val);
}

void SRFInt::parse(std::istream &is) {
	val = readInt(is);
}

void SRFFloat::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::FLOAT);
	writeFloat(os, val);
}

void SRFFloat::parse(std::istream &is) {
	val = readFloat(is);
}

void SRFDouble::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::DOUBLE);
	writeDouble(os, val);
}

void SRFDouble::parse(std::istream &is) {
	val = readDouble(is);
}

void SRFNone::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::NONE);
}

void SRFNone::parse(std::istream &is) {}

void SRFByteArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::BYTE_ARRAY);
	writeInt(os, val.size());
	os.write((const char *)val.data(), val.size());
}

void SRFByteArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);
	is.read((char *)val.data(), count);
}

void SRFWordArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::WORD_ARRAY);
	writeInt(os, val.size());

	for (auto &v: val) {
		writeWord(os, v);
	}
}

void SRFWordArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);

	for (int32_t i = 0; i < count; ++i) {
		val[i] = readWord(is);
	}
}

void SRFIntArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::INT_ARRAY);
	writeInt(os, val.size());

	for (auto &v: val) {
		writeInt(os, v);
	}
}

void SRFIntArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);

	for (int32_t i = 0; i < count; ++i) {
		val[i] = readInt(is);
	}
}

void SRFFloatArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::FLOAT_ARRAY);
	writeInt(os, val.size());

	for (auto &v: val) {
		writeFloat(os, v);
	}
}

void SRFFloatArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);

	for (int32_t i = 0; i < count; ++i) {
		val[i] = readFloat(is);
	}
}

void SRFDoubleArray::serialize(std::ostream &os) {
	writeByte(os, (uint8_t)Type::DOUBLE_ARRAY);
	writeInt(os, val.size());

	for (auto &v: val) {
		writeDouble(os, v);
	}
}

void SRFDoubleArray::parse(std::istream &is) {
	int32_t count = readInt(is);
	val.resize(count);

	for (int32_t i = 0; i < count; ++i) {
		val[i] = readDouble(is);
	}
}

}
