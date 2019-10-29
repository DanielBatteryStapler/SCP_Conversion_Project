#include "AutomatedTester.hpp"

namespace Tests{
    void Tester::add(std::string name, std::function<void()> func){
        tests.push_back(std::make_pair(name, func));
    }
    
    void Tester::runTests(){
        unsigned int passedTests = 0;
        for(unsigned int i = 0; i < tests.size(); i++){
            std::string name = tests[i].first;
            std::function<void()> func = tests[i].second;
            std::cout << "\nRunning test \"" << name << "\"...\n";
            bool passed = true;
            try{
                func();
            }
            catch(std::exception& e){
                std::cout << "Test \"" << name << "\" failed, Error:\"" << e.what() << "\"\n";
                passed = false;
            }
            if(passed){
                std::cout << "Test \"" << name << "\" passed\n";
                passedTests++;
            }
        }
        std::cout << "\n\n||=====================================||\n"
        << "  Summary:\n"
        << "        " << passedTests << " out of " << tests.size() << " tests passed.\n"
        ;
        if(passedTests == tests.size()){
            std::cout << "\n        All tests passed!\n";
        }
        else{
            throw std::runtime_error("Not all tests passed");
        }
    }
    
    void shouldThrowException(std::function<void()> func){
        try{
            func();
        }
        catch(std::exception& e){
            return;//return with no error
        }
        throw std::runtime_error("Expected a thrown exception, but got none");
    }

    void assertTrue(bool assertion){
        if(assertion == false){
            throw std::runtime_error("Assertion Error");
        }
    }
    
    void addTesterTests(Tester& tester){
        tester.add("Tests::shouldThrowException", [](){
            shouldThrowException([](){shouldThrowException([](){});});
            try{
                shouldThrowException([](){});
            }
            catch(std::exception& e){
                return;
            }
            throw std::runtime_error("Expected \"shouldThrowException\" to throw an exception, but it didn't");
        });
    }
}
