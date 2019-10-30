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
	
	template<typename A>
    inline void assertEqualsVec(std::vector<A> expected, std::vector<A> actual){
		bool areEqual = true;
		if(expected.size() == actual.size()){
			for(int i = 0; i < expected.size(); i++){
				if(!(expected[i] == actual[i])){
					areEqual = false;
					break;
				}
			}
		}
		else{
			areEqual = false;
		}
		if(!areEqual){
			std::stringstream ss;
			ss << "Assert Fail, expected the following std::vectors to be equal:\n"
			<< "\tExpected: {";
			for(int i = 0; i < expected.size(); i++){
				ss << expected[i];
				if(i != expected.size() - 1){
					ss << ", ";
				}
			}
			ss << "}\n\tActual:   {";
			for(int i = 0; i < actual.size(); i++){
				ss << actual[i];
				if(i != actual.size() - 1){
					ss << ", ";
				}
			}
			ss << "}\n";
			throw std::runtime_error(ss.str());
		}
	}
    
    void addTesterTests(Tester& tester);
}

#endif // AUTOMATEDTESTER_H
