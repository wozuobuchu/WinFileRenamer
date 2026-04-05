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
#include <filesystem>

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

		virtual std::unique_ptr<Element> clone() = 0;

		virtual const std::wstring get_str() const {
			throw std::runtime_error("Cant transform this class into wstring !");
			return std::wstring{};
		}
	};

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
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Str>(*this); }

		virtual const std::wstring get_str() const override { return str; }

		friend std::wostream& operator<<(std::wostream& out, const Str& s) {
			out << s.str;
			return out;
		}

	};

	class Int64_Format final : public Element {
	public:
		Int64_Format() = default;
		Int64_Format(int64_t minimumLength) { data = minimumLength; }
		virtual ~Int64_Format() {}
		constexpr int64_t get_type() override { return 'F'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Int64_Format>(*this); }
		int64_t get_min_length() const { return data; }
	};

	class Lbracket final : public Element {
	public:
		Lbracket() = default;
		virtual ~Lbracket() {}
		constexpr virtual int64_t get_type() override { return '('; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Lbracket>(*this); }
	};

	class Rbracket final : public Element {
	public:
		Rbracket() = default;
		virtual ~Rbracket() {}
		constexpr virtual int64_t get_type() override { return ')'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Rbracket>(*this); }
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
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Int64>(*this); }

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

		using OptFunc = std::function<std::unique_ptr<Element>(const std::unique_ptr<Element>&,const std::unique_ptr<Element>&)>;

	protected:
		static constexpr uint32_t make_key(int64_t type1, int64_t type2) noexcept {
			return (static_cast<uint32_t>(type1 & 0xFF) << 8) | static_cast<uint32_t>(type2 & 0xFF);
		}

		static uint32_t make_key(const std::unique_ptr<Element>& a, const std::unique_ptr<Element>& b) noexcept {
			return make_key(a->get_type(), b->get_type());
		}

		template <typename T>
		static const T* as(const std::unique_ptr<Element>& p) {
			return static_cast<const T*>(p.get());
		}

		template <typename L, typename R, typename Fn>
		static OptFunc bind(Fn&& fn) {
			return [f = std::forward<Fn>(fn)] (const std::unique_ptr<Element>& a, const std::unique_ptr<Element>& b) -> std::unique_ptr<Element> {
					return f(as<L>(a), as<R>(b));
			};
		}

		static std::unique_ptr<Element> dispatch_or_throw(
			const std::unordered_map<uint32_t, OptFunc>& table,
			const std::unique_ptr<Element>& a,
			const std::unique_ptr<Element>& b,
			const char* optName
		) {

			auto it = table.find(make_key(a, b));
			if (it == table.end()) {
				std::stringstream ss;
				ss << "Illegal operator type \"" << optName << "\" !";
				throw std::runtime_error(ss.str());
			}
			return it->second(a, b);
		}

	public:
		virtual std::unique_ptr<Element> do_opt(std::unique_ptr<Element> ptr1, std::unique_ptr<Element> ptr2) = 0;
	};

	class Add_Int64Opt final : public Int64Opt {
	public:
		Add_Int64Opt() { data = 4; }
		virtual ~Add_Int64Opt() {}
		constexpr virtual int64_t get_opt_type() override { return '+'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Add_Int64Opt>(*this); }

	private:
		static const std::unordered_map<uint32_t, OptFunc>& table() {
			static const std::unordered_map<uint32_t, OptFunc> t {
				// Z + Z -> Z
				{ make_key('Z', 'Z'), bind<Int64, Int64>([](const Int64* a, const Int64* b) {
					return std::make_unique<Int64>((*a) + (*b));
				}) },

				// Z + S / S + Z / S + S -> S (string concat)
				{ make_key('Z', 'S'), bind<Int64, Str>([](const Int64* a, const Str* b) {
					return std::make_unique<Str>(a->get_str() + b->get_str());
				}) },
				{ make_key('S', 'Z'), bind<Str, Int64>([](const Str* a, const Int64* b) {
					return std::make_unique<Str>(a->get_str() + b->get_str());
				}) },
				{ make_key('S', 'S'), bind<Str, Str>([](const Str* a, const Str* b) {
					return std::make_unique<Str>(a->get_str() + b->get_str());
				}) },
			};
			return t;
		}

	public:
		virtual std::unique_ptr<Element> do_opt(std::unique_ptr<Element> ptr1, std::unique_ptr<Element> ptr2) override {
			return dispatch_or_throw(table(), ptr1, ptr2, "Add");
		}

		static void prewarm_table() {
			(void)table();
		}
	};

	class Sub_Int64Opt final : public Int64Opt {
	public:
		Sub_Int64Opt() { data = 4; }
		virtual ~Sub_Int64Opt() {}
		constexpr virtual int64_t get_opt_type() override { return '-'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Sub_Int64Opt>(*this); }

	private:
		static const std::unordered_map<uint32_t, OptFunc>& table() {
			static const std::unordered_map<uint32_t, OptFunc> t{
				{ make_key('Z', 'Z'), bind<Int64, Int64>([](const Int64* a, const Int64* b) {
					return std::make_unique<Int64>((*a) - (*b));
				}) },
			};
			return t;
		}

	public:
		virtual std::unique_ptr<Element> do_opt(std::unique_ptr<Element> ptr1, std::unique_ptr<Element> ptr2) override {
			return dispatch_or_throw(table(), ptr1, ptr2, "Sub");
		}

		static void prewarm_table() {
			(void)table();
		}
	};

	class Mul_Int64Opt final : public Int64Opt {
	public:
		Mul_Int64Opt() { data = 3; }
		virtual ~Mul_Int64Opt() {}
		constexpr virtual int64_t get_opt_type() override { return '*'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Mul_Int64Opt>(*this); }

	private:
		static int64_t cnt_num_len(int64_t n) {
			// handle negative + INT64_MIN safely
			uint64_t u = 0;
			if (n < 0) {
				u = static_cast<uint64_t>(-(n + 1)) + 1ULL;
			} else {
				u = static_cast<uint64_t>(n);
			}

			if (u == 0) return 1;
			int64_t cnt = 0;
			while (u > 0) {
				u /= 10;
				++cnt;
			}

			return cnt;
		}

		static std::unique_ptr<Element> format_with_min_len(
			const Int64* num_ptr,
			const Int64_Format* fmt_ptr) {

			std::wstringstream wss;

			int64_t expected_len = fmt_ptr->get_min_length();
			int64_t num_len = cnt_num_len(num_ptr->get_val());

			int64_t pad = expected_len - num_len;
			if (pad < 0) pad = 0;
			for (int64_t i = 0; i < pad; ++i) wss << L"0";
			wss << num_ptr->get_val();

			return std::make_unique<Str>(wss.str());
		}

		static const std::unordered_map<uint32_t, OptFunc>& table() {
			static const std::unordered_map<uint32_t, OptFunc> t{
				// Z * Z -> Z
				{ make_key('Z', 'Z'), bind<Int64, Int64>([](const Int64* a, const Int64* b) {
					return std::make_unique<Int64>((*a) * (*b));
				}) },

				// Z * F / F * Z -> S (number formatting)
				{ make_key('Z', 'F'), bind<Int64, Int64_Format>([](const Int64* a, const Int64_Format* b) {
					return format_with_min_len(a, b);
				}) },
				{ make_key('F', 'Z'), bind<Int64_Format, Int64>([](const Int64_Format* a, const Int64* b) {
					return format_with_min_len(b, a);
				}) },
			};
			return t;
		}

	public:
		virtual std::unique_ptr<Element> do_opt(std::unique_ptr<Element> ptr1, std::unique_ptr<Element> ptr2) override {
			return dispatch_or_throw(table(), ptr1, ptr2, "Mul");
		}

		static void prewarm_table() {
			(void)table();
		}
	};

	class Div_Int64Opt final : public Int64Opt {
	public:
		Div_Int64Opt() { data = 3; }
		virtual ~Div_Int64Opt() {}
		constexpr virtual int64_t get_opt_type() override { return '/'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Div_Int64Opt>(*this); }

	private:
		static const std::unordered_map<uint32_t, OptFunc>& table() {
			static const std::unordered_map<uint32_t, OptFunc> t{
				{ make_key('Z', 'Z'), bind<Int64, Int64>([](const Int64* a, const Int64* b) {
					return std::make_unique<Int64>((*a) / (*b));
				}) },
			};
			return t;
		}

	public:
		virtual std::unique_ptr<Element> do_opt(std::unique_ptr<Element> ptr1, std::unique_ptr<Element> ptr2) override {
			return dispatch_or_throw(table(), ptr1, ptr2, "Div");
		}

		static void prewarm_table() {
			(void)table();
		}
	};

	class Var : public Element {
	public:
		Var() = default;
		virtual ~Var() {}
		constexpr int64_t get_type() override { return 'X'; }
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Var>(*this); }
		constexpr virtual int64_t get_var_type() { return 'X'; }
	};

	class Index_Var final : public Var {
	public:
		Index_Var() = default;
		virtual ~Index_Var() {}
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<Index_Var>(*this); }
		constexpr int64_t get_var_type() override { return 'I'; }
	};

	class OriginFileName_Var final : public Var {
	public:
		OriginFileName_Var() = default;
		virtual ~OriginFileName_Var() {}
		virtual std::unique_ptr<Element> clone() override { return std::make_unique<OriginFileName_Var>(*this); }
		constexpr int64_t get_var_type() override { return 'N'; }
	};


	std::vector<std::unique_ptr<calc::Element>> generate_rpn(const std::vector<std::unique_ptr<calc::Element>>& expr) {
		std::vector<std::unique_ptr<calc::Element>> ret;
		std::stack<std::unique_ptr<calc::Element>> stk;

		size_t obj_cnt = 0;
		size_t opt_cnt = 0;

		for (auto& ptr : expr) {
			int64_t type = ptr->get_type();
			if (type == 'Z' || type == 'S' || type == 'X' || type == 'F') {
				ret.emplace_back(ptr->clone());
				++obj_cnt;
			} else if (type == '(') {
				stk.push(ptr->clone());
			} else if (type == ')') {
				bool flag = false;
				std::unique_ptr<calc::Element> top_ptr;
				while (!stk.empty()) {
					top_ptr = std::move(stk.top());
					stk.pop();
					int64_t top_type = top_ptr->get_type();
					if (top_type == '(') {
						flag = true;
						break;
					} else if (top_type == ')') {
						flag = false;
						break;
					} else if (top_type == '#') {
						ret.emplace_back(std::move(top_ptr));
						++opt_cnt;
					}
				}

				if (flag == false) throw std::runtime_error("Match bracket failed !");

			} else if (type == '#') {
				calc::Int64Opt* now_ptr = static_cast<calc::Int64Opt*>(ptr.get());

				while (!stk.empty()) {
					auto& general_top_ptr = stk.top();
					if (general_top_ptr->get_type() == '(') break;

					if (general_top_ptr->get_type() == '#') {
						calc::Int64Opt* top_ptr = static_cast<calc::Int64Opt*>(general_top_ptr.get());
						if (*now_ptr < *top_ptr) {
							break;
						} else {
							ret.emplace_back(std::move(stk.top()));
							++opt_cnt;
							stk.pop();
						}
					} else {
						throw std::runtime_error("Unexpected element on operator stack !");
					}
				}

				stk.push(ptr->clone());
			}
		}

		while (!stk.empty()) {
			auto top_ptr = std::move(stk.top());
			int64_t top_type = top_ptr->get_type();

			if (top_type == '(' || top_type == ')') throw std::runtime_error("Match bracket failed !");

			ret.emplace_back(std::move(top_ptr));
			++opt_cnt;
			stk.pop();
		}

		if (obj_cnt > opt_cnt + 1) throw std::runtime_error("Missing operator !");
		if (obj_cnt < opt_cnt + 1) throw std::runtime_error("Exceeding operator !");

		return ret;
	}




	std::vector<std::unique_ptr<calc::Element>> preprocess_rpn(const std::vector<std::unique_ptr<calc::Element>>& rpn, int64_t var_index, const std::wstring& fname) {
		std::vector<std::unique_ptr<calc::Element>> ret;

		for (auto& ptr : rpn) {
			int64_t type = ptr->get_type();
			if (type == 'X') {
				auto var_ptr = static_cast<calc::Var*>(ptr.get());
				int64_t var_type = var_ptr->get_var_type();
				if (var_type == 'I') {
					ret.emplace_back(std::make_unique<calc::Int64>(var_index));
				} else if (var_type == 'N') {
					std::filesystem::path ofp = fname;
					ret.emplace_back(std::make_unique<calc::Str>(ofp.filename().wstring()));
				} else {
					throw std::runtime_error("Unknown variable type in RPN !");
				}
			} else {
				ret.emplace_back(ptr->clone());
			}
		}

		return ret;
	}




	std::wstring calculate_rpn(const std::vector<std::unique_ptr<calc::Element>>& rpn) {
		std::stack<std::unique_ptr<calc::Element>> stk;

		for (auto& ptr : rpn) {
			int64_t type = ptr->get_type();
			if (type == 'Z' || type == 'S' || type == 'F') {
				stk.push(ptr->clone());

			} else if (type == '#') {
				if (stk.size() < 2) throw std::runtime_error("Illegal expression !");

				auto v_ptr = std::move(stk.top());
				stk.pop();
				auto u_ptr = std::move(stk.top());
				stk.pop();

				auto opt_ptr = static_cast<Int64Opt*>(ptr.get());

				stk.push(opt_ptr->do_opt(std::move(u_ptr), std::move(v_ptr)));

			} else {
				throw std::runtime_error("Illegal data type in preprocessed RPN !");

			}


		}

		if (stk.size() != 1) throw std::runtime_error("Illegal expression !");

		auto top_ptr = std::move(stk.top());
		int64_t top_type = top_ptr->get_type();
		std::wstringstream ss;

		if (top_type == 'Z') ss << (*static_cast<Int64*>(top_ptr.get()));
		else if (top_type == 'S') ss << (*static_cast<Str*>(top_ptr.get()));
		else throw std::runtime_error("Illegal data type in preprocessed RPN !");

		return ss.str();
	}

	// Call this once during program startup to pre-initialize operator dispatch tables.
	inline void warmup_operator_tables() {
		Add_Int64Opt::prewarm_table();
		Sub_Int64Opt::prewarm_table();
		Mul_Int64Opt::prewarm_table();
		Div_Int64Opt::prewarm_table();
	}


} // namespace calc

#endif