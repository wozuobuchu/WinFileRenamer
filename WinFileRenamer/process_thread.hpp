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
#include <unordered_map>

namespace pt {

static std::wstring MakeLongPath(const std::wstring& path) {
	// If already in long path format, return as is
	if (path.rfind(L"\\\\?\\", 0) == 0) {
		return path;
	}

	// Handle UNC paths
	if (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\') {
		// \\server\share\xxx  ->  \\?\UNC\server\share\xxx
		return L"\\\\?\\UNC\\" + path.substr(2);
	}

	// Handle drive letter paths
	if (path.size() >= 2 && path[1] == L':') {
		return L"\\\\?\\" + path;
	}

	// For other paths, return as is (could be relative paths)
	return path;
}

class ProcessThread {
public:
	static constexpr int STATE_READY = 0;
	static constexpr int STATE_ONGOING = 1;

private:
	std::atomic<bool> msg_box_;

	std::atomic<int> state_;

	std::mutex vec_filepath_cache_mtx;
	std::vector<std::wstring> vec_filepath_cache;

	std::mutex input_expr_ptr_mtx;
	std::shared_ptr<std::vector<std::shared_ptr<calc::Element>>> input_expr_ptr;

	std::mutex res_wstr_mtx;
	std::wstring res_wstr;

	inline static std::wstring old_dir;

	std::thread rename_thread;
	void rename_thread_assist() {
		state_.store(STATE_ONGOING, std::memory_order_release);

		std::vector<std::wstring> vec_filepath;
		std::vector<std::wstring> vec_newname;

		{
			std::lock_guard<std::mutex> lck(vec_filepath_cache_mtx);
			vec_filepath = vec_filepath_cache;
		}

		bool calc_flag = false;
		try {
			std::lock_guard<std::mutex> lck(input_expr_ptr_mtx);
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
				std::lock_guard<std::mutex> lck(res_wstr_mtx);
				res_wstr = wss.str();
			}
		}

		bool rename_flag = false;
		size_t vsize = vec_filepath.size();
		if (calc_flag) {
			try {
				for (size_t i = 0; i < vsize; ++i) {

					std::filesystem::path src(MakeLongPath(vec_filepath[i]));
					std::filesystem::path dst(MakeLongPath(vec_newname[i]));

					if (!std::filesystem::exists(src)) throw std::runtime_error("File doesn't exist !");
					if (std::filesystem::exists(dst)) throw std::runtime_error("Target file already exists !");

					std::filesystem::rename(src, dst);
				}
				rename_flag = true;
			} catch (const std::filesystem::filesystem_error& e) {
				std::wstringstream wss;
				wss << e.what();
				{
					std::lock_guard<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			} catch (const std::runtime_error& re) {
				std::wstringstream wss;
				wss << re.what();
				{
					std::lock_guard<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			} catch (...) {
				std::wstringstream wss;
				wss << "Unknown Error !";
				{
					std::lock_guard<std::mutex> lck(res_wstr_mtx);
					res_wstr = wss.str();
				}
			}
		}

		if (rename_flag && calc_flag) {
			std::wstringstream wss;
			wss << L"Successfully renamed " << vsize << L" files.";
			{
				std::lock_guard<std::mutex> lck(res_wstr_mtx);
				res_wstr = wss.str();
			}
		}

		state_.store(STATE_READY, std::memory_order_release);
		msg_box_.store(true, std::memory_order_release);
		
	}

	inline static std::unordered_map< int64_t, std::function< std::wstring(const std::shared_ptr<calc::Element>&) > > func_umap {
		{
			'S',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				auto str_ptr = std::static_pointer_cast<calc::Str>(elem);
				return L"\"" + str_ptr->get_str() + L"\" ";
			}
		},
		{
			'Z',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				auto int_ptr = std::static_pointer_cast<calc::Int64>(elem);
				return int_ptr->get_str() + L" ";
			}
		},
		{
			'X',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				auto var_ptr = std::static_pointer_cast<calc::Var>(elem);
				int64_t var_type = var_ptr->get_var_type();
				if (var_type == 'I') {
					return L"INDEX ";
				} else if (var_type == 'N') {
					return L"OFNAME ";
				} else {
					throw std::runtime_error("Unknown variable type in expression !");
				}
			}
		},
		{
			'(',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				return L"( ";
			}
		},
		{
			')',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				return L") ";
			}
		},
		{
			'#',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				auto opt_ptr = std::static_pointer_cast<calc::Int64Opt>(elem);
				std::wstringstream wss;
				wss << (wchar_t)opt_ptr->get_opt_type() << L" ";
				return wss.str();
			}
		},
		{
			'F',
			[](const std::shared_ptr<calc::Element>& elem) -> std::wstring {
				auto fptr = std::static_pointer_cast<calc::Int64_Format>(elem);
				std::wstringstream wss;
				wss << L"NUM_FORMAT_" << fptr->get_min_length() << L" ";
				return wss.str();
			}
		},
	};

public:

	static void set_old_dir(const std::wstring& dir) {
		old_dir = MakeLongPath(dir);
	}

	static std::wstring& get_old_dir() {
		return old_dir;
	}

	bool process_lunch() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;
		if (rename_thread.joinable()) rename_thread.join();
		rename_thread = std::thread(std::bind(&ProcessThread::rename_thread_assist, this));
		return true;
	}

	ProcessThread() {
		msg_box_.store(false, std::memory_order_release);
		state_.store(STATE_READY, std::memory_order_release);
		// Initialize the expression pointer.
		input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
	}

	virtual ~ProcessThread() {
		if (rename_thread.joinable()) {
			rename_thread.join();
		}
	}

	bool reset_selected_file() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			std::lock_guard<std::mutex> lck(vec_filepath_cache_mtx);
			vec_filepath_cache.clear();
		}

		return true;
	}

	bool push_filepath(const std::wstring& filepath) {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			std::lock_guard<std::mutex> lck(vec_filepath_cache_mtx);
			vec_filepath_cache.emplace_back(filepath);
		}

		return true;
	}

	bool reset_input_expr_ptr() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			std::lock_guard<std::mutex> lck(input_expr_ptr_mtx);
			// Check before clearing, or re-create if null
			if (input_expr_ptr) {
				input_expr_ptr->clear();
			}
			else {
				input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
			}
		}

		return true;
	}

	bool pop_expr_ptr() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			std::lock_guard<std::mutex> lck(input_expr_ptr_mtx);
			// Check for valid pointer and non-empty vector
			if (input_expr_ptr && !input_expr_ptr->empty()) {
				input_expr_ptr->pop_back();
			}
		}

		return true;
	}

	template <typename PtrType, typename... Args>
	bool push_expr(Args&&... args) {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			std::lock_guard<std::mutex> lck(input_expr_ptr_mtx);
			// Check in case pointer was reset
			if (!input_expr_ptr) {
				input_expr_ptr = std::make_shared<std::vector<std::shared_ptr<calc::Element>>>();
			}
			input_expr_ptr->emplace_back(std::make_shared<PtrType>(std::forward<Args>(args)...));
		}

		return true;
	}

	std::wstring get_res_wstr() {
		std::lock_guard<std::mutex> lck(res_wstr_mtx);
		return res_wstr;
	}

	int get_state() {
		return state_.load(std::memory_order_acquire);
	}

	int get_and_clear_msg_box() {
		return msg_box_.exchange(false, std::memory_order_acq_rel);
	}

	std::wstring get_expression_str() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return L"?";

		std::lock_guard<std::mutex> lck(input_expr_ptr_mtx);
		if (!input_expr_ptr) {
			return L"";
		}

		std::wstringstream rewss;
		for (const auto& elem : *input_expr_ptr) {
			int64_t type = elem->get_type();
			try {
				rewss << func_umap.at(type)(elem);
			} catch (...) {
				rewss << L"UNKNOWN ERROR? ";
			}


		}

		return rewss.str();
	}

	void join() {
		if(rename_thread.joinable()) {
			rename_thread.join();
		}
	}
};

} // namespace pt

#endif // !_PROCESS_THREAD_HPP