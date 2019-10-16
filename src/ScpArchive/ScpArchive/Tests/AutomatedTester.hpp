#ifndef AUTOMATEDTESTER_H
#define AUTOMATEDTESTER_H

#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <sstream>

namespace Tests{
    class Tester{
		public:
			void add(std::string name, std::function<void()> func);
			void runTests();
			
		private:
			std::vector<std::pair<std::string, std::function<void()>>> tests;
    };

    void shouldThrowException(std::function<void()> func);
    
	void assertTrue(bool assertion);
    
    template<typename A, typename B>
    inline void assertEquals(A expected, B actual){
		if(!(expected == actual)){
			std::stringstream ss;
			ss << "Assert Fail, expected the following to be equal:\n"
			<< "\tExpected: " << expected << "\n"
			<< "\tActual:   " << actual   << "\n";
			throw std::runtime_error(ss.str());
		}
	}
    
    void addTesterTests(Tester& tester);
}

#endif // AUTOMATEDTESTER_H
