#ifndef _PROCESS_THREAD_HPP
#define _PROCESS_THREAD_HPP

#include "calc.hpp"
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

namespace pt {

class ProcessThread {
private:

	std::shared_mutex vec_filepath_cache_smtx;
	std::vector<std::wstring> vec_filepath_cache;

	std::shared_mutex input_expr_ptr_smtx;
	std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> input_expr_ptr;

	std::thread rename_thread;
public:
	ProcessThread() = default;
	virtual ~ProcessThread() {
		if (rename_thread.joinable()) {
			rename_thread.join();
		}
	}

	void reset_selected_file() {
		calc::var_index = 0;
		{
			std::unique_lock<std::shared_mutex> lck(vec_filepath_cache_smtx);
			vec_filepath_cache.clear();
		}
	}

	void reset_input_expr_ptr() {
		std::unique_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
		input_expr_ptr->clear();
		input_expr_ptr.reset();
	}

	void push_filepath(const std::wstring& filepath) {
		std::unique_lock<std::shared_mutex> lck(vec_filepath_cache_smtx);
		vec_filepath_cache.emplace_back(filepath);
	}

	template <typename PtrType, typename... Args>
	void push_expr(Args&&... args) {
		std::unique_lock<std::shared_mutex> lck(input_expr_ptr_smtx);
		input_expr_ptr->emplace_back(std::make_shared<PtrType>(std::forward<Args>(args)...));
	}

};

}

#endif // !_PROCESS_THREAD_HPP
