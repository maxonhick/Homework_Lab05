## Laboratory work V

[![Coverage Status](https://coveralls.io/repos/github/maxonhick/Homework_Lab05/badge.svg)](https://coveralls.io/github/maxonhick/Homework_Lab05)  


## Homework

### Задание
1. Создайте `CMakeList.txt` для библиотеки *banking*.
2. Создайте модульные тесты на классы `Transaction` и `Account`.
    * Используйте mock-объекты.
    * Покрытие кода должно составлять 100%.
3. Настройте сборочную процедуру на **TravisCI**.
4. Настройте [Coveralls.io](https://coveralls.io/).

Создадим директорию. Скопируем файлы из лабораторной работы в наш репозиторий.
```
mkdir lab05
cd lab05
git clone https://github.com/tp-labs/lab05 
git remote remove origin
git remote add origin https://github.com/BridgeInSky/lab05_home
git add .
git commit -m "lab05"
git push origin master
```
Создадим папку third-party и добавим туда gtest
```
mkdir third-party
git submodule add https://github.com/google/googletest third-party/gtest
```
Добавим CMakeLists.txt в главную папку и напишем его
```
touch CMakeLists.txt && nano CMakeLists.txt
```
Текст главного CMake
```
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

enable_testing()

project(banking)

set(Sources
    banking/Account.cpp
    banking/Transaction.cpp
)
set(Headers
    banking/Account.h
    banking/Transaction.h
)

add_library(banking STATIC ${Sources} ${Headers})
target_include_directories(banking PUBLIC banking)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(third-party/gtest)
    add_executable(my_test tests/tests.cpp)
    target_link_libraries(my_test
        gtest_main
        gmock_main
        banking
    )
    add_test(NAME my_test COMMAND my_test)
endif()
```
Добавим заранее указанную папку 'tests', а в ней файл tests.cpp
```
touch tests/tests.cpp
code tests/tests.cpp
```
Текст тестов
```
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Account.h"
#include "Transaction.h"

using ::testing::Return;
using ::testing::Throw;
using ::testing::_;

class MyAccount : public Account {
public:
    MyAccount(int id, int balance) : Account(id, balance) {}
    MOCK_CONST_METHOD0(GetBalance, int());
    MOCK_METHOD1(ChangeBalance, void(int diff));
    MOCK_METHOD0(Lock, void());
    MOCK_METHOD0(Unlock, void());
};

// Я подумал, что использование Gmock, нужно, но не везде, поэтому Transaction без него

TEST(Account, Locker) {
    MyAccount acc(0, 1111);
	EXPECT_CALL(acc, Lock()).Times(2);
	EXPECT_CALL(acc, Unlock()).Times(1);
	acc.Lock();
	acc.Lock();
	acc.Unlock();
}

TEST(Account, balance_positive) {
    Account acc1(0, 1000);
    EXPECT_EQ(acc1.GetBalance(), 1000);

    acc1.Lock();
    EXPECT_NO_THROW(acc1.ChangeBalance(100));

    EXPECT_EQ(acc1.GetBalance(), 1100);
}

TEST(Accout, balance_negative) {
    Account Vasya(1, 100);

    EXPECT_THROW(Vasya.ChangeBalance(100), std::runtime_error);
    
    Vasya.Lock();
    EXPECT_ANY_THROW(Vasya.Lock());
}

TEST(Transaction, construnct_and_positive) {
    Transaction first;
    EXPECT_EQ(first.fee(), 1);

    Account Petya(0, 6132);
    Account Katya(1, 2133);

    first.set_fee(32);
    EXPECT_EQ(first.fee(), 32);

    EXPECT_TRUE(first.Make(Petya, Katya, 100));
    EXPECT_EQ(Katya.GetBalance(), 2233);
    EXPECT_EQ(Petya.GetBalance(), 6000);

}

TEST(Transaction, negative) {
    Transaction second;
    second.set_fee(51);
    Account Roma(0, 10);
    Account Misha(1, 1000);

    EXPECT_THROW(second.Make(Misha, Misha, 0), std::logic_error);

    EXPECT_THROW(second.Make(Misha, Roma, -100), std::invalid_argument);

    EXPECT_THROW(second.Make(Misha, Roma, 50), std::logic_error);

    EXPECT_FALSE(second.Make(Misha, Roma, 100));

    second.set_fee(10);

    EXPECT_FALSE(second.Make(Roma, Misha, 100));
    
}
```
Была исправлена небольшая ошибка в самом коде программы, где деньги списывались не у того
```
bool success = Debit(to, sum + fee_);
bool success = Debit(from, sum + fee_);
```
Добавим папку coverage, а в ней lcov.info файл
```
touch coverage/lcov.info
```
Остолось только добавить yml файл
```
touch .github/workflows/lab.yml
```
Текст yml файла
```
name: lab_actions

on:
 push:
  branches: [master]
 pull_request:
  branches: [master]

jobs: 
 build_Linux:

  runs-on: ubuntu-latest

  steps:
  - uses: actions/checkout@v4

  - name: putting gtest
    run: git clone https://github.com/google/googletest.git third-party/gtest
    

  - name: Install lcov
    run: sudo apt-get install -y lcov 
  
  - name: Configurate
    run: |
      rm -rf ${{github.workspace}}/_build
      mkdir _build && cd _build
      cmake .. -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS='--coverage'
      cmake --build .

  - name: Run tests
    run: _build/my_test
      
  - name: lcov
    run: lcov -c -d _build/CMakeFiles/banking.dir/banking/ --include *.cpp --output-file ./coverage/lcov.info
  
  - name: Coveralls
    uses: coverallsapp/github-action@v2
    with:
      github-token: ${{ secrets.GITHUB_TOKEN }} 
      path-to-lcov: ${{ github.workspace }}/coverage/lcov.info
```
