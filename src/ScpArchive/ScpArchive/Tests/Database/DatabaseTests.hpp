#ifndef DATABASETESTS_HPP
#define DATABASETESTS_HPP

#include "../AutomatedTester.hpp"

namespace Tests{

	static inline const std::string testDatabaseName = "TemporaryTestingDatabaseFile";
	
	void addDatabaseTests(Tester& tester);
}

#endif // DATABASETESTS_HPP
