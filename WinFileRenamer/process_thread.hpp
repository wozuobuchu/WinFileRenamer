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

namespace pt {

class ProcessThread {
public:
	std::atomic<bool> msg_box_;

	static constexpr int STATE_READY = 0;
	static constexpr int STATE_ONGOING = 1;

private:
	std::atomic<int> state_;

	std::shared_mutex vec_filepath_cache_smtx;
	std::vector<std::wstring> vec_filepath_cache;

	std::shared_mutex input_expr_ptr_smtx;
	std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> input_expr_ptr;

	std::shared_mutex res_wstr_smtx;
	std::wstring res_wstr;

	std::thread rename_thread;
	void rename_thread_assist() {
		state_.store(STATE_ONGOING);

		std::vector<std::wstring> vec_filepath;
		std::vector<std::wstring> vec_newname;

		{
			std::shared_lock<std::shared_mutex> lck(vec_filepath_cache_smtx);
			vec_filepath = vec_filepath_cache;
		}

		calc::var_index = 0;

		bool calc_flag = true;
		try {
			std::shared_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
			auto rpn_ptr = calc::generate_rpn(input_expr_ptr);
			for (const auto& filepath : vec_filepath) vec_newname.emplace_back(calc::calculate_rpn(calc::preprocess_rpn(rpn_ptr)));
		} catch (std::runtime_error& re) {
			std::wstringstream wss;
			wss << re.what();
			{
				std::unique_lock<std::shared_mutex> lck(res_wstr_smtx);
				res_wstr = wss.str();
			}
			calc_flag = false;
		}

		bool rename_flag = true;
		if (calc_flag) {

		}


		state_.store(STATE_READY);
		msg_box_.store(true);
	}

public:

	void process_lunch() {

	}

	ProcessThread() {
		msg_box_.store(false);
		state_.store(STATE_READY);
	}

	virtual ~ProcessThread() {
		if (rename_thread.joinable()) {
			rename_thread.join();
		}
	}

	void reset_selected_file() {
		{
			std::unique_lock<std::shared_mutex> lck(res_wstr_smtx);
			res_wstr.clear();
		}
		{
			std::unique_lock<std::shared_mutex> lck(vec_filepath_cache_smtx);
			vec_filepath_cache.clear();
		}
	}

	void push_filepath(const std::wstring& filepath) {
		std::unique_lock<std::shared_mutex> lck(vec_filepath_cache_smtx);
		vec_filepath_cache.emplace_back(filepath);
	}

	void reset_input_expr_ptr() {
		std::unique_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
		input_expr_ptr->clear();
		input_expr_ptr.reset();
	}

	void pop_expr_ptr() {
		std::unique_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
		if (!input_expr_ptr->empty()) {
			input_expr_ptr->pop_back();
		}
	}

	template <typename PtrType, typename... Args>
	void push_expr(Args&&... args) {
		std::unique_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
		input_expr_ptr->emplace_back(std::make_shared<PtrType>(std::forward<Args>(args)...));
	}

	std::wstring get_res_wstr() {
		std::shared_lock<std::shared_mutex> lck(res_wstr_smtx);
		return res_wstr;
	}

	int get_state() {
		return state_.load();
	}

};

} // namespace pt

#endif // !_PROCESS_THREAD_HPP
