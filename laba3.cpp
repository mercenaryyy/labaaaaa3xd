#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <locale>
#include <sstream>
#include <limits>

class Client {
protected:
    std::string name;
    double baseRate;

public:
    Client(const std::string& name, double baseRate)
        : name(name), baseRate(baseRate) {
    }

    virtual ~Client() = default;

    const std::string& getName() const { return name; }
    double getBaseRate() const { return baseRate; }

    virtual double calculateTotalCost() const = 0;
    virtual std::string getInfo() const {
        return "Клиент: " + name + ", Базовая ставка: " + std::to_string(baseRate);
    }
};

class DiscountStrategy {
public:
    virtual ~DiscountStrategy() = default;
    virtual double applyDiscount(double baseCost) const = 0;
    virtual std::string getStrategyName() const = 0;
};

class FixedDiscount : public DiscountStrategy {
private:
    double discountAmount;
public:
    FixedDiscount(double amount) : discountAmount(amount) {
        if (amount < 0) {
            throw std::invalid_argument("Фиксированная скидка не может быть отрицательной.");
        }
    }
    double applyDiscount(double baseCost) const override {
        double discounted = baseCost - discountAmount;
        return (discounted > 0) ? discounted : 0.0;
    }
    std::string getStrategyName() const override {
        return "Фиксированная скидка: " + std::to_string(discountAmount);
    }
};

class PercentageDiscount : public DiscountStrategy {
private:
    double discountPercent;
public:
    PercentageDiscount(double percent) : discountPercent(percent) {
        if (percent < 0 || percent > 100) {
            throw std::invalid_argument("Процент скидки должен быть в диапазоне [0, 100].");
        }
    }
    double applyDiscount(double baseCost) const override {
        return baseCost * (1.0 - discountPercent / 100.0);
    }
    std::string getStrategyName() const override {
        return "Процентная скидка: " + std::to_string(discountPercent) + "%";
    }
};

class DiscountedClient : public Client {
private:
    std::unique_ptr<DiscountStrategy> discountStrategy;

public:
    DiscountedClient(const std::string& name, double baseRate, std::unique_ptr<DiscountStrategy> strategy)
        : Client(name, baseRate), discountStrategy(std::move(strategy)) {
        if (!discountStrategy) {
            throw std::invalid_argument("Стратегия скидки не может быть nullptr.");
        }
    }

    double calculateTotalCost() const override {
        return discountStrategy->applyDiscount(baseRate);
    }

    std::string getInfo() const override {
        return Client::getInfo() + ", Стратегия: " + discountStrategy->getStrategyName()
            + ", Итоговая стоимость: " + std::to_string(calculateTotalCost());
    }
};

class InternetOperator {
private:
    std::vector<std::unique_ptr<Client>> clients;

public:
    void addClient(std::unique_ptr<Client> client) {
        if (!client) {
            throw std::invalid_argument("Нельзя добавить пустого клиента.");
        }
        clients.push_back(std::move(client));
    }

    double calculateTotalCost() const {
        double total = 0.0;
        for (const auto& client : clients) {
            total += client->calculateTotalCost();
        }
        return total;
    }

    void printAllClients() const {
        try {
            std::locale::global(std::locale(""));
        }
        catch (...) {}

        if (clients.empty()) {
            std::cout << "\nСписок клиентов пуст. Добавьте клиентов через пункт 1.\n\n";
            return;
        }
        std::cout << "\n=== Текущий список ВСЕХ клиентов ===" << std::endl;
        for (size_t i = 0; i < clients.size(); ++i) {
            std::cout << (i + 1) << ". " << clients[i]->getInfo() << std::endl;
        }
        std::cout << "====================================\n" << std::endl;
    }

    bool isEmpty() const {
        return clients.empty();
    }
};

template<typename T>
T inputNumber(const std::string& prompt) {
    T value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        else {
            std::cout << "Ошибка: введите корректное число.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void addClientMenu(InternetOperator& op) {
    try {
        std::locale::global(std::locale(""));
    }
    catch (...) {}

    std::cout << "\n--- Добавление нового клиента ---\n";
    std::cout << "Введите имя клиента: ";
    std::cin.ignore();
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) {
        std::cout << "Ошибка: имя не может быть пустым.\n\n";
        return;
    }

    double baseRate = inputNumber<double>("Введите базовую стоимость услуг (в рублях): ");
    if (baseRate < 0) {
        std::cout << "Ошибка: базовая стоимость не может быть отрицательной.\n\n";
        return;
    }

    std::cout << "Выберите тип скидки:\n";
    std::cout << "1. Фиксированная скидка (в рублях)\n";
    std::cout << "2. Процентная скидка (в %)\n";
    int choice = inputNumber<int>("Ваш выбор (1 или 2): ");

    try {
        if (choice == 1) {
            double amount = inputNumber<double>("Введите сумму фиксированной скидки (руб): ");
            auto client = std::make_unique<DiscountedClient>(
                name, baseRate, std::make_unique<FixedDiscount>(amount)
            );
            op.addClient(std::move(client));
            std::cout << "Клиент  успешно добавлен!\n\n";
        }
        else if (choice == 2) {
            double percent = inputNumber<double>("Введите процент скидки (0–100): ");
            auto client = std::make_unique<DiscountedClient>(
                name, baseRate, std::make_unique<PercentageDiscount>(percent)
            );
            op.addClient(std::move(client));
            std::cout << "Клиент успешно добавлен!\n\n";
        }
        else {
            std::cout << "Неверный выбор. Клиент не добавлен.\n\n";
        }
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n\n";
    }
}

void showMenu() {
    std::cout << "=== Меню Интернет-оператора ===\n";
    std::cout << "1. Добавить клиента\n";
    std::cout << "2. Показать ВСЕХ клиентов\n";
    std::cout << "3. Показать суммарную стоимость услуг\n";
    std::cout << "4. Выйти из программы\n";
    std::cout << "================================\n";
}

static int inputInt(const std::string& prompt, int min = 0, int max = 1000000) {
    int value;
    std::string line;

    while (true) {
        std::cout << prompt;
        std::getline(std::cin, line);

        std::istringstream iss(line);
        if (iss >> value) {
            char remaining;
            if (!(iss >> remaining)) {
                if (value >= min && value <= max) {
                    return value;
                }
            }
        }

        std::cout << "Ошибка ввода! Введите целое число от " << min << " до " << max << "\n";
    }
}

static double inputDouble(const std::string& prompt, double min = 0.0, int max = 200) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << "Ошибка ввода! Введите число от " << min << " до " << max << "\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int main() {
    system("chcp 1251 > nul");

    try {
        std::locale::global(std::locale(""));
    }
    catch (...) {}

    InternetOperator operator_;
    auto client1 = std::make_unique<DiscountedClient>(
        "Анна Петрова",
        1500.0,
        std::make_unique<FixedDiscount>(200.0)
    );
    auto client2 = std::make_unique<DiscountedClient>(
        "Иван Сидоров",
        2000.0,
        std::make_unique<PercentageDiscount>(15.0)
    );
    auto client3 = std::make_unique<DiscountedClient>(
        "Елена Кузнецова",
        1800.0,
        std::make_unique<FixedDiscount>(100.0)
    );

    operator_.addClient(std::move(client1));
    operator_.addClient(std::move(client2));
    operator_.addClient(std::move(client3));
    int choice;

    std::cout << "Добро пожаловать в систему управления клиентами Интернет-оператора!\n\n";

    while (true) {
        showMenu();
        choice = inputInt("Выберите действие (1–4): ", 1, 4);

        switch (choice) {
        case 1:
            addClientMenu(operator_);
            break;
        case 2:
            operator_.printAllClients();
            break;
        case 3: {
            if (operator_.isEmpty()) {
                std::cout << "\nНет клиентов. Суммарная стоимость: 0 руб.\n\n";
            }
            else {
                double total = operator_.calculateTotalCost();
                std::cout << "\nСуммарная стоимость услуг для всех клиентов: " << total << " руб.\n\n";
            }
            break;
        }
        case 4:
            std::cout << "\nСпасибо за использование программы! До свидания.\n";
            return 0;
        default:
            std::cout << "\nНеверный выбор. Пожалуйста, введите число от 1 до 4.\n\n";
        }
    }

    return 0;
}