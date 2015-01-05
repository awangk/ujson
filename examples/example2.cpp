/*
 * Copyright (c) 2014 Anders Wang Kristensen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ujson/ujson.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>

struct employee {

    employee(std::string fn, std::string ln, double b, bool m)
        : first_name(std::move(fn)), last_name(std::move(ln)),
          accumulated_bonus(b), married(m) {}

    explicit employee(ujson::value value) {

        auto object = object_cast(std::move(value));

        first_name = string_cast(at(object, "first_name")->second);
        first_name = string_cast(at(object, "first_name")->second);

        last_name = string_cast(at(object, "last_name")->second);
        accumulated_bonus =
            double_cast(at(object, "accumulated_bonus")->second);
        married = bool_cast(at(object, "married")->second);
    }

    std::string first_name;
    std::string last_name;
    double accumulated_bonus;
    bool married;

    friend bool operator==(employee const &lhs, employee const &rhs) {
        return lhs.first_name == rhs.first_name &&
               lhs.last_name == rhs.last_name &&
               lhs.accumulated_bonus == rhs.accumulated_bonus &&
               lhs.married == rhs.married;
    }
};

ujson::value to_json(employee const &e) {
    return ujson::object{ { "first_name", e.first_name },
                          { "last_name", e.last_name },
                          { "accumulated_bonus", e.accumulated_bonus },
                          { "married", e.married } };
}

struct company {
    company(std::string n, double r, std::vector<employee> e,
            std::map<std::string, double> b)
        : name(std::move(n)), revenue(r), employees(std::move(e)),
          branch_revenues(std::move(b)) {}

    explicit company(ujson::value value) {

        auto object = object_cast(std::move(value));

        name = string_cast(at(object, "name")->second);
        revenue = double_cast(at(object, "revenue")->second);

        auto employee_values = array_cast(at(object, "employees")->second);
        employees.reserve(employee_values.size());
        for (auto &value : employee_values)
            employees.push_back(employee(std::move(value)));

        auto branch_values =
            object_cast(at(object, "branch_revenues")->second);
        for (auto pair : branch_values)
            branch_revenues.insert({ pair.first, double_cast(pair.second) });
    }

    std::string name;
    double revenue;
    std::vector<employee> employees;
    std::map<std::string, double> branch_revenues;

    friend bool operator==(company const &lhs, company const &rhs) {
        return lhs.name == rhs.name && lhs.revenue == rhs.revenue &&
               lhs.employees == rhs.employees &&
               lhs.branch_revenues == rhs.branch_revenues;
    }
};

ujson::value to_json(company const &c) {
    return ujson::object{ { "name", c.name },
                          { "revenue", c.revenue },
                          { "employees", c.employees },
                          { "branch_revenues", c.branch_revenues } };
}

int main(int argc, const char *argv[]) {

    try {
        const company c0{ "My Company",
                          3.12e6,
                          { { "Michael", "Madsen", 123.32, false },
                            { "John", "Jensen", 657.12, true } },
                          { { "Los Angeles", 1.06e6 },
                            { "San Diego", 2.06e6 } } };

        // convert object to string rep
        auto string = to_string(to_json(c0));

        std::cout << string << std::endl;

        // parse string rep and reconstruct object
        auto c1 = company(ujson::parse(string));

        if (c0 == c1)
            std::cout << "Success!" << std::endl;
        else
            std::cout << "Failure!" << std::endl;

        return EXIT_SUCCESS;
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
