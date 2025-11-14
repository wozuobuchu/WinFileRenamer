#ifndef _PROCESS_THREAD_HPP
#define _PROCESS_THREAD_HPP

#include "calc.hpp"
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include <cstdint>
#include <filesystem>

namespace pt {

class ProcessThread {
public:
	std::atomic<bool> msg_box_;

	static constexpr int STATE_READY = 0;
	static constexpr int STATE_ONGOING = 1;

private:
	std::atomic<int> state_;

	std::mutex vec_filepath_cache_mtx;
	std::vector<std::wstring> vec_filepath_cache;

	std::mutex input_expr_ptr_mtx;
	std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> input_expr_ptr;

	std::mutex res_wstr_mtx;
	std::wstring res_wstr;

	std::thread rename_thread;
	void rename_thread_assist() {
		state_.store(STATE_ONGOING);

		std::vector<std::wstring> vec_filepath;
		std::vector<std::wstring> vec_newname;

		{
			std::unique_lock<std::mutex> lck(vec_filepath_cache_mtx);
			vec_filepath = vec_filepath_cache;
		}

		bool calc_flag = false;
		try {
			std::unique_lock<std::mutex> lck(input_expr_ptr_mtx);
			// Check if pointer is valid before generating
			if (!input_expr_ptr) throw std::runtime_error("Expression is empty!");
			auto rpn_ptr = calc::generate_rpn(input_expr_ptr);
			
			for (size_t var_idex = 0; var_idex < vec_filepath.size(); ++var_idex) {
				const std::wstring& ofname = vec_filepath[var_idex];
				vec_newname.emplace_back(calc::calculate_rpn(calc::preprocess_rpn(rpn_ptr, var_idex, ofname)));
			}

			calc_flag = true;
		} catch (const std::runtime_error& re) {
			std::wstringstream wss;
			wss << re.what();
			{
				std::unique_lock<std::mutex> lck(res_wstr_mtx);
				res_wstr = wss.str();
			}
		}

		bool rename_flag = false;
		size_t vsize = vec_filepath.size();
		if (calc_flag) {
			try {
				size_t vsize = vec_filepath.size();
				for (size_t i = 0; i < vsize; ++i) {
					if (!std::filesystem::exists(vec_filepath[i])) throw std::runtime_error("File doesn't exist !");
					if (std::filesystem::exists(vec_newname[i])) throw std::runtime_error("Target file already exists !");
					std::filesystem::rename(vec_filepath[i], vec_newname[i]);
				}
				rename_flag = true;
			} catch (const std::filesystem::filesystem_error& e) {
				std::wstringstream wss;
				wss << e.what();
				{
					std::unique_lock<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			} catch (const std::runtime_error& re) {
				std::wstringstream wss;
				wss << re.what();
				{
					std::unique_lock<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			} catch (...) {
				std::wstringstream wss;
				wss << "Unknown Error !";
				{
					std::unique_lock<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			}
		}

		if (rename_flag && calc_flag) {
			std::wstringstream wss;
			wss << L"Successfully renamed " << vsize << L" files.";
			{
				std::unique_lock<std::mutex> lck(res_wstr_mtx);
				res_wstr = wss.str();
			}
		}

		state_.store(STATE_READY);
		msg_box_.store(true);
	}

public:

	bool process_lunch() {
		if (state_.load() == STATE_ONGOING) return false;
		if (rename_thread.joinable()) rename_thread.join();
		rename_thread = std::thread(std::bind(&ProcessThread::rename_thread_assist, this));
		return true;
	}

	ProcessThread() {
		msg_box_.store(false);
		state_.store(STATE_READY);
		// Initialize the expression pointer.
		input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
	}

	virtual ~ProcessThread() {
		if (rename_thread.joinable()) {
			rename_thread.join();
		}
	}

	void reset_selected_file() {
		{
			std::unique_lock<std::mutex> lck(vec_filepath_cache_mtx);
			vec_filepath_cache.clear();
		}
	}

	void push_filepath(const std::wstring& filepath) {
		std::unique_lock<std::mutex> lck(vec_filepath_cache_mtx);
		vec_filepath_cache.emplace_back(filepath);
	}

	void reset_input_expr_ptr() {
		std::unique_lock<std::mutex> lck(input_expr_ptr_mtx);
		// Check before clearing, or re-create if null
		if (input_expr_ptr) {
			input_expr_ptr->clear();
		} else {
			input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
		}
	}

	void pop_expr_ptr() {
		std::unique_lock<std::mutex> lck(input_expr_ptr_mtx);
		// Check for valid pointer and non-empty vector
		if (input_expr_ptr && !input_expr_ptr->empty()) {
			input_expr_ptr->pop_back();
		}
	}

	template <typename PtrType, typename... Args>
	void push_expr(Args&&... args) {
		std::unique_lock<std::mutex> lck(input_expr_ptr_mtx);
		// Check in case pointer was reset
		if (!input_expr_ptr) {
			input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
		}
		input_expr_ptr->emplace_back(std::make_shared<PtrType>(std::forward<Args>(args)...));
	}

	std::wstring get_res_wstr() {
		std::unique_lock<std::mutex> lck(res_wstr_mtx);
		return res_wstr;
	}

	int get_state() {
		return state_.load();
	}

	std::wstring get_expression_str() {
		std::unique_lock<std::mutex> lck(input_expr_ptr_mtx);
		if (!input_expr_ptr) {
			return L"";
		}

		std::wstringstream wss;
		for (const auto& elem : *input_expr_ptr) {
			int64_t type = elem->get_type();
			try {
				if (type == 'S') {
					wss << L"\"" << elem->get_str() << L"\" ";
				} else if (type == 'Z') {
					wss << elem->get_str() << L" ";
				} else if (type == 'X') {
					auto var_ptr = std::static_pointer_cast<calc::Var>(elem);
					int64_t var_type = var_ptr->get_var_type();
					if (var_type == 'I') {
						wss << L"INDEX ";
					} else if (var_type == 'F') {
						wss << L"OFNAME ";
					} else {
						wss << L"VAR ";
					}
				} else if (type == '(') {
					wss << L"( ";
				} else if (type == ')') {
					wss << L") ";
				} else if (type == '#') {
					auto opt_ptr = std::static_pointer_cast<calc::Int64Opt>(elem);
					wss << (wchar_t)opt_ptr->get_opt_type() << L" ";
				}
			} catch (...) {
				wss << L"?ERROR? ";
			}
		}
		return wss.str();
	}

	void join() {
		if(rename_thread.joinable()) {
			rename_thread.join();
		}
	}
};

} // namespace pt

#endif // !_PROCESS_THREAD_HPP