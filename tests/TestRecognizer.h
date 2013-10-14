#include <cppunit/extensions/HelperMacros.h>

class TestRecognizer : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TestRecognizer);
	CPPUNIT_TEST(testRecognizePlateNumber);
	CPPUNIT_TEST_SUITE_END();
protected:
	void testRecognizePlateNumber();
private:
	bool threshold(const char* guess, const char* exact);
};