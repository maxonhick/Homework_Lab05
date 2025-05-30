#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Account.h"
#include "Transaction.h"

using ::testing::Return;
using ::testing::_;
using ::testing::AtLeast;

// Mock класс для Account
class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

// Тесты для Account
TEST(Account, GetBalance_ReturnsCorrectValue) {
    Account account(1, 1000);
    EXPECT_EQ(account.GetBalance(), 1000);
}

TEST(Account, ChangeBalance_UpdatesBalance) {
    Account account(1, 1000);
    account.Lock();
    account.ChangeBalance(500);
    EXPECT_EQ(account.GetBalance(), 1500);
}

TEST(Account, Lock_Unlock_WorksCorrectly) {
    Account account(1, 1000);
    account.Lock();
    EXPECT_THROW(account.Lock(), std::runtime_error);
    account.Unlock();
    EXPECT_NO_THROW(account.Lock());
}

// Тесты для Transaction
TEST(Transaction, Make_CallsAccountMethodsCorrectly) {
    // Создаем mock-объекты для аккаунтов
    MockAccount from(1, 1000);
    MockAccount to(2, 500);

    // Настраиваем ожидания для методов
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(from, ChangeBalance(-110)).Times(1);
    EXPECT_CALL(to, ChangeBalance(100)).Times(1);
    EXPECT_CALL(from, GetBalance()).Times(AtLeast(1)).WillRepeatedly(Return(1000));
    EXPECT_CALL(to, GetBalance()).Times(AtLeast(1)).WillRepeatedly(Return(500));

    // Выполняем транзакцию
    Transaction transaction;
    transaction.set_fee(10);
    EXPECT_TRUE(transaction.Make(from, to, 100));
}

TEST(Transaction, Make_ThrowsExceptionIfSumIsNegative) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);

    Transaction transaction;
    EXPECT_THROW(transaction.Make(from, to, -100), std::invalid_argument);
}

TEST(Transaction, Make_ThrowsExceptionIfSumIsTooSmall) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);

    Transaction transaction;
    EXPECT_THROW(transaction.Make(from, to, 50), std::logic_error);
}

TEST(Transaction, Make_ReturnsFalseIfFeeIsTooHigh) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);

    Transaction transaction;
    transaction.set_fee(1000);
    EXPECT_FALSE(transaction.Make(from, to, 100));
}

TEST(Transaction, Make_SavesToDatabaseCorrectly) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);

    // Настраиваем ожидания для всех вызовов методов
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(from, ChangeBalance(_)).Times(1);
    EXPECT_CALL(to, ChangeBalance(_)).Times(1);
    EXPECT_CALL(from, GetBalance()).Times(AtLeast(1)).WillRepeatedly(Return(1000));
    EXPECT_CALL(to, GetBalance()).Times(AtLeast(1)).WillRepeatedly(Return(500));

    // Выполняем транзакцию
    Transaction transaction;
    transaction.set_fee(10);
    EXPECT_TRUE(transaction.Make(from, to, 100));
}

TEST(Transaction, Make_ReturnsFalseIfInsufficientFunds) {
    MockAccount from(1, 100); // На счету 100
    MockAccount to(2, 500);   // На счету 500

    // Настраиваем ожидания для вызовов
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(from, GetBalance())
        .WillOnce(Return(100))  // Первый вызов
        .WillOnce(Return(100)); // Второй вызов
    EXPECT_CALL(to, GetBalance())
        .WillOnce(Return(500)); // Первый вызов
    EXPECT_CALL(to, ChangeBalance(200)).Times(1);           // Вызов ChangeBalance для получателя
    EXPECT_CALL(from, ChangeBalance(-200 - 10)).Times(0);   // Не должно быть вызова для отправителя
    EXPECT_CALL(to, ChangeBalance(-200)).Times(1);          // Откат ChangeBalance для получателя

    Transaction transaction;
    transaction.set_fee(10); // Устанавливаем комиссию 10

    // Пытаемся перевести 200, что больше, чем на счету
    EXPECT_FALSE(transaction.Make(from, to, 200));
}