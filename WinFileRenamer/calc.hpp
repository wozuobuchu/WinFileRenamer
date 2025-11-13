#ifndef _CALC_HPP
#define _CALC_HPP

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <queue>
#include <cstdint>
#include <utility>
#include <functional>
#include <array>
#include <memory>
#include <stack>
#include <string>
#include <exception>
#include <sstream>

#include <thread>
#include <chrono>

#include <iostream>

namespace calc {

class Element {
protected:
    int64_t data;
public:
    Element() { data = 0; }
    virtual ~Element() {}
    constexpr virtual int64_t get_type() = 0;

    virtual std::shared_ptr<Element> clone() = 0;

    virtual const std::wstring get_str() const {
        throw std::runtime_error("Cant transform this class into wstring !");
        return std::wstring{};
    }
};

class Str;
class Int64;

class Str final : public Element {
private:
    friend class Int64;
    friend class Int64Opt;
    friend class Add_Int64Opt;
    std::wstring str;
public:
    Str() = default;
    Str(int64_t x) {
        std::wstringstream ss;
        ss << x;
        ss >> str;
    }
    Str(const std::wstring& cpp_string) : str(cpp_string) {}
    Str(std::wstring&& cpp_string) : str(std::move(cpp_string)) {}
    Str(const Str& other) : str(other.str) {}
    Str(Str&& other) noexcept { str = std::move(other.str); }

    Str& operator=(const std::wstring& cpp_string) {
        str = cpp_string;
        return *this;
    }
    Str& operator=(std::wstring&& cpp_string) {
        str = std::move(cpp_string);
        return *this;
    }
    Str& operator=(const Str& other) {
        str = other.str;
        return *this;
    }
    Str& operator=(Str&& other) noexcept {
        str = std::move(other.str);
        return *this;
    }

    virtual ~Str() {}
    constexpr virtual int64_t get_type() { return 'S'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Str>(*this); }

    virtual const std::wstring get_str() const override { return str; }

    friend std::wostream& operator<<(std::wostream& out, const Str& s) {
        out << s.str;
        return out;
    }

};

class Lbracket final : public Element {
public:
    Lbracket() = default;
    virtual ~Lbracket() {}
    constexpr virtual int64_t get_type() override { return '('; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Lbracket>(*this); }
};

class Rbracket final : public Element {
public:
    Rbracket() = default;
    virtual ~Rbracket() {}
    constexpr virtual int64_t get_type() override { return ')'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Rbracket>(*this); }
};

class Int64 final : public Element {
private:
    friend class Str;
    friend class Int64Opt;
    friend class Add_Int64Opt;
    friend class Sub_Int64Opt;
    friend class Mul_Int64Opt;
    friend class Div_Int64Opt;
public:
    Int64() = default;
    Int64(int64_t i) { data = i; }
    virtual ~Int64() {}
    constexpr virtual int64_t get_type() override { return 'Z'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Int64>(*this); }

    int64_t get_val() const { return data; }

    bool operator==(const Int64& other) const { return data == other.data; }
    bool operator<(const Int64& other) const { return data < other.data; }
    bool operator<=(const Int64& other) const { return data <= other.data; }
    bool operator>(const Int64& other) const { return data > other.data; }
    bool operator>=(const Int64& other) const { return data >= other.data; }

    Int64 operator+(const Int64& other) const { return Int64(data + other.data); }
    Int64 operator-(const Int64& other) const { return Int64(data - other.data); }
    Int64 operator*(const Int64& other) const { return Int64(data * other.data); }
    Int64 operator/(const Int64& other) const { return (other.data == 0) ? Int64(0x7fffffffffffffff) : Int64(data / other.data); }

    friend std::wostream& operator<<(std::wostream& out, const Int64& i) {
        out << i.data;
        return out;
    }

    virtual const std::wstring get_str() const override {
        std::wstringstream wss;
        wss << data;
        return wss.str();
    }

};

class Int64Opt : public Element {
public:
    Int64Opt() = default;
    virtual ~Int64Opt() {}
    constexpr virtual int64_t get_type() override { return '#'; }

    constexpr virtual int64_t get_opt_type() { return '#'; }

    bool operator==(const Int64Opt& other) const { return data == other.data; }
    bool operator<(const Int64Opt& other) const { return data < other.data; }
    bool operator<=(const Int64Opt& other) const { return data <= other.data; }
    bool operator>(const Int64Opt& other) const { return data > other.data; }
    bool operator>=(const Int64Opt& other) const { return data >= other.data; }

    virtual std::shared_ptr<Element> do_opt(std::shared_ptr<Element> ptr1, std::shared_ptr<Element> ptr2) = 0;
};

class Add_Int64Opt final : public Int64Opt {
public:
    Add_Int64Opt() { data = 4; }
    virtual ~Add_Int64Opt() {}
    constexpr virtual int64_t get_opt_type() override { return '+'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Add_Int64Opt>(*this); }

    virtual std::shared_ptr<Element> do_opt(std::shared_ptr<Element> ptr1, std::shared_ptr<Element> ptr2) override {
        int64_t type1 = ptr1->get_type();
        int64_t type2 = ptr2->get_type();
        if (type1 == 'Z' && type2 == 'Z') {
            std::shared_ptr<Int64> res = std::make_shared<Int64>(*(std::static_pointer_cast<Int64>(ptr1)) + *(std::static_pointer_cast<Int64>(ptr2)));
            return std::static_pointer_cast<Element>(res);
        } else if ((type1 == 'Z' && type2 == 'S') || (type1 == 'S' && type2 == 'Z') || (type1 == 'S' && type2 == 'S')) {
            std::shared_ptr<Str> res = std::make_shared<Str>(std::move(ptr1->get_str() + ptr2->get_str()));
            return std::static_pointer_cast<Element>(res);
        }

        throw std::runtime_error("Illegal operator type \"Add\" !");

        return nullptr;
    }

};

class Sub_Int64Opt final : public Int64Opt {
public:
    Sub_Int64Opt() { data = 4; }
    virtual ~Sub_Int64Opt() {}
    constexpr virtual int64_t get_opt_type() override { return '-'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Sub_Int64Opt>(*this); }

    virtual std::shared_ptr<Element> do_opt(std::shared_ptr<Element> ptr1, std::shared_ptr<Element> ptr2) override {
        int64_t type1 = ptr1->get_type();
        int64_t type2 = ptr2->get_type();
        if (type1 == 'Z' && type2 == 'Z') {
            std::shared_ptr<Int64> res = std::make_shared<Int64>(*(std::static_pointer_cast<Int64>(ptr1)) - *(std::static_pointer_cast<Int64>(ptr2)));
            return std::static_pointer_cast<Element>(res);
        }

        throw std::runtime_error("Illegal operator type \"Sub\" !");

        return nullptr;
    }
};

class Mul_Int64Opt final : public Int64Opt {
public:
    Mul_Int64Opt() { data = 3; }
    virtual ~Mul_Int64Opt() {}
    constexpr virtual int64_t get_opt_type() override { return '*'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Mul_Int64Opt>(*this); }

    virtual std::shared_ptr<Element> do_opt(std::shared_ptr<Element> ptr1, std::shared_ptr<Element> ptr2) override {
        int64_t type1 = ptr1->get_type();
        int64_t type2 = ptr2->get_type();
        if (type1 == 'Z' && type2 == 'Z') {
            std::shared_ptr<Int64> res = std::make_shared<Int64>(*(std::static_pointer_cast<Int64>(ptr1)) * *(std::static_pointer_cast<Int64>(ptr2)));
            return std::static_pointer_cast<Element>(res);
        }

        throw std::runtime_error("Illegal operator type \"Mul\" !");

        return nullptr;
    }
};

class Div_Int64Opt final : public Int64Opt {
public:
    Div_Int64Opt() { data = 3; }
    virtual ~Div_Int64Opt() {}
    constexpr virtual int64_t get_opt_type() override { return '/'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Div_Int64Opt>(*this); }

    virtual std::shared_ptr<Element> do_opt(std::shared_ptr<Element> ptr1, std::shared_ptr<Element> ptr2) override {
        int64_t type1 = ptr1->get_type();
        int64_t type2 = ptr2->get_type();
        if (type1 == 'Z' && type2 == 'Z') {
            std::shared_ptr<Int64> res = std::make_shared<Int64>(*(std::static_pointer_cast<Int64>(ptr1)) / *(std::static_pointer_cast<Int64>(ptr2)));
            return std::static_pointer_cast<Element>(res);
        }

        throw std::runtime_error("Illegal operator type \"Div\"!");

        return nullptr;
    }
};

class Var : public Element {
public:
    Var() = default;
    virtual ~Var() {}
    constexpr int64_t get_type() override { return 'X'; }
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Var>(*this); }
    constexpr virtual int64_t get_var_type() { return 'X'; }
};

class Index_Var final : public Var {
public:
    Index_Var() = default;
    virtual ~Index_Var() {}
    virtual std::shared_ptr<Element> clone() override { return std::make_shared<Index_Var>(*this); }
    constexpr int64_t get_var_type() override { return 'I'; }
};




std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> generate_rpn(std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> expr_ptr) {
    const std::vector<std::shared_ptr<calc::Element>>& expr = *expr_ptr;
    std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> ret = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();

    std::stack<std::shared_ptr<calc::Element>> stk;

    size_t obj_cnt = 0;
    size_t opt_cnt = 0;

    for (auto ptr : expr) {
        int64_t type = ptr->get_type();
        if (type == 'Z' || type == 'S' || type == 'X') {
            ret->emplace_back(ptr);
            ++obj_cnt;

        } else if (type == '(') {
            stk.push(ptr);

        } else if (type == ')') {
            bool flag = false;
            std::shared_ptr<calc::Element> top_ptr;
            while (!stk.empty()) {
                top_ptr = stk.top();
                stk.pop();
                int64_t top_type = top_ptr->get_type();
                if (top_type == '(') {
                    flag = true;
                    break;
                }
                else if (top_type == ')') {
                    flag = false;
                    break;
                }
                else if (top_type == '#') {
                    ret->emplace_back(top_ptr);
                    ++opt_cnt;
                }
            }

            if (flag == false) throw std::runtime_error("Match bracket failed !");

        } else if (type == '#') {
            std::shared_ptr<calc::Int64Opt> now_ptr = std::static_pointer_cast<calc::Int64Opt>(ptr);
            std::shared_ptr<calc::Int64Opt> top_ptr;

            while (!stk.empty()) {
                auto general_top_ptr = stk.top();
                if (general_top_ptr->get_type() == '(') break;

                if (general_top_ptr->get_type() == '#') {
                    std::shared_ptr<calc::Int64Opt> top_ptr = std::static_pointer_cast<calc::Int64Opt>(general_top_ptr);
                    if (*now_ptr < *top_ptr) {
                        break;
                    } else {
                        ret->emplace_back(top_ptr);
                        ++opt_cnt;
                        stk.pop();
                    }
                } else {
                    throw std::runtime_error("Unexpected element on operator stack !");
                }
            }

            stk.push(now_ptr);
        }
    }

    while (!stk.empty()) {
        auto top_ptr = stk.top();
        int64_t top_type = top_ptr->get_type();

        if (top_type == '(' || top_type == ')') throw std::runtime_error("Match bracket failed !");

        ret->emplace_back(top_ptr);
        ++opt_cnt;
        stk.pop();
    }

    if (obj_cnt > opt_cnt + 1) throw std::runtime_error("Missing operator !");
    if (obj_cnt < opt_cnt + 1) throw std::runtime_error("Exceeding operator !");

    return ret;
}




int64_t var_index = 0;
std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> preprocess_rpn(std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> rpn_ptr) {
    auto ret = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
    const std::vector<std::shared_ptr<calc::Element>>& rpn = *rpn_ptr;

    for (auto ptr : rpn) {
        int64_t type = ptr->get_type();
        if (type == 'X') {
            auto var_ptr = std::static_pointer_cast<calc::Var>(ptr);
            if (var_ptr->get_var_type() == 'I') {
                ret->emplace_back(std::make_shared<calc::Int64>(var_index));
            } else {
                throw std::runtime_error("Unknown variable type in RPN !");
            }
        } else {
            ret->emplace_back(ptr);
        }
    }

    ++var_index;

    return ret;
}




std::wstring calculate_rpn(std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> rpn_ptr) {
    const std::vector<std::shared_ptr<calc::Element>>& rpn = *rpn_ptr;

    std::stack<std::shared_ptr<calc::Element>> stk;

    for (auto ptr : rpn) {
        int64_t type = ptr->get_type();
        if (type == 'Z' || type == 'S') {
            stk.push(ptr);

        } else if (type == '#') {
            if (stk.size() < 2) throw std::runtime_error("Illegal expression !");

            auto v_ptr = stk.top();
            stk.pop();
            auto u_ptr = stk.top();
            stk.pop();

            auto opt_ptr = std::static_pointer_cast<Int64Opt>(ptr);

            stk.push(opt_ptr->do_opt(u_ptr, v_ptr));

        } else {
            throw std::runtime_error("Illegal data type in preprocessed RPN !");

        }


    }

    if (stk.size() != 1) throw std::runtime_error("Illegal expression !");

    auto top_ptr = stk.top();
    int64_t top_type = top_ptr->get_type();
    std::wstringstream ss;

    if (top_type == 'Z') ss << (*std::static_pointer_cast<Int64>(top_ptr));
    else if (top_type == 'S') ss << (*std::static_pointer_cast<Str>(top_ptr));
    else throw std::runtime_error("Illegal data type in preprocessed RPN !");

    return ss.str();
}





    
} // namespace calc

#endif