#ifndef _PROCESS_THREAD_HPP
#define _PROCESS_THREAD_HPP

#include "aop.hpp"
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
#include <algorithm>

namespace pt {

inline static std::wstring MakeLongPath(const std::wstring& path) {
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

	aop::LockBox<std::vector<std::wstring>> vec_filepath_cache;

	aop::LockBox<std::vector<std::unique_ptr<calc::Element>>> input_expr;

	aop::LockBox<std::wstring> res_wstr;

	inline static std::wstring old_dir;

	std::thread rename_thread;

	void rename_thread_assist_expr() {
		state_.store(STATE_ONGOING, std::memory_order_release);

		std::vector<std::wstring> vec_filepath;
		std::vector<std::wstring> vec_newname;

		{
			auto lck = vec_filepath_cache.AcquireLock();
			vec_filepath = *lck;
		}

		bool calc_flag = false;
		try {
			auto lck = input_expr.AcquireLock();
			// Check if pointer is valid before generating
			if (lck->empty()) throw std::runtime_error("Expression is empty!");
			std::vector<std::unique_ptr<calc::Element>> rpn = calc::generate_rpn(*lck);

			for (size_t var_idex = 0; var_idex < vec_filepath.size(); ++var_idex) {
				const std::wstring& ofname = vec_filepath[var_idex];
				std::wstring new_filename = calc::calculate_rpn(calc::preprocess_rpn(rpn, var_idex, ofname));
				std::filesystem::path src_path(ofname);
				std::filesystem::path dst_path = src_path.parent_path() / new_filename;
				vec_newname.emplace_back(dst_path.wstring());
			}

			calc_flag = true;
		} catch (const std::runtime_error& re) {
			std::wstringstream wss;
			wss << re.what();
			{
				auto lck = res_wstr.AcquireLock();
				*lck = wss.str();
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
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			} catch (const std::runtime_error& re) {
				std::wstringstream wss;
				wss << re.what();
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			} catch (...) {
				std::wstringstream wss;
				wss << "Unknown Error !";
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			}
		}

		if (rename_flag && calc_flag) {
			std::wstringstream wss;
			wss << L"Successfully renamed " << vsize << L" files.";
			{
				auto lck = res_wstr.AcquireLock();
				*lck = wss.str();
			}
		}

		state_.store(STATE_READY, std::memory_order_release);
		msg_box_.store(true, std::memory_order_release);
		
	}

	void rename_thread_assist_auto() {
		state_.store(STATE_ONGOING, std::memory_order_release);

		std::vector<std::wstring> vec_filepath;
		std::vector<std::wstring> vec_newname;
		{
			auto lck = vec_filepath_cache.AcquireLock();
			vec_filepath = *lck;
		}

		std::vector<std::wstring> video_files;
		std::vector<std::wstring> subtitle_files;
		std::vector<std::wstring> other_files;

		for (auto& path : vec_filepath) {
			path = MakeLongPath(path);

			std::wstring ext = std::filesystem::path(path).extension().wstring();
			for (auto& c : ext) {
				if (c >= L'A' && c <= L'Z') c = c - L'A' + L'a';
			}

			if (ext == L".mp4" || ext == L".mkv" || ext == L".avi" || ext == L".wmv" || ext == L".mov" || ext == L".flv") {
				video_files.push_back(path);
			} else if (ext == L".srt" || ext == L".ass" || ext == L".ssa" || ext == L".vtt") {
				subtitle_files.push_back(path);
			} else {
				other_files.push_back(path);
			}
		}

		if (video_files.size() != subtitle_files.size()) {
			std::wstringstream wss;
			wss << L"Failed: Number of video files and subtitle files do not match.";
			{
				auto lck = res_wstr.AcquireLock();
				*lck = wss.str();
			}
		} else {
			std::sort(video_files.begin(), video_files.end());
			std::sort(subtitle_files.begin(), subtitle_files.end());

			try {
				for (size_t i = 0; i < video_files.size(); ++i) {
					std::filesystem::path v_path(video_files[i]);
					std::filesystem::path s_path(subtitle_files[i]);

					std::filesystem::path new_s_path = v_path;
					new_s_path.replace_extension(s_path.extension());

					if (!std::filesystem::exists(s_path)) throw std::runtime_error("Subtitle file doesn't exist !");
					if (std::filesystem::exists(new_s_path) && s_path != new_s_path) throw std::runtime_error("Target subtitle file already exists !");

					if (s_path != new_s_path) {
						std::filesystem::rename(s_path, new_s_path);
					}
				}

				std::wstringstream wss;
				wss << L"Successfully renamed " << subtitle_files.size() << L" subtitles.";
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			} catch (const std::filesystem::filesystem_error& e) {
				std::wstringstream wss;
				wss << e.what();
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			} catch (const std::runtime_error& re) {
				std::wstringstream wss;
				wss << re.what();
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			} catch (...) {
				std::wstringstream wss;
				wss << L"Unknown Error !";
				{
					auto lck = res_wstr.AcquireLock();
					*lck = wss.str();
				}
			}
		}

		state_.store(STATE_READY, std::memory_order_release);
		msg_box_.store(true, std::memory_order_release);
	}

	inline static std::unordered_map< int64_t, std::function< std::wstring(const std::unique_ptr<calc::Element>&) > > func_umap {
		{
			'S',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				auto str_ptr = static_cast<calc::Str*>(elem.get());
				return L"\"" + str_ptr->get_str() + L"\" ";
			}
		},
		{
			'Z',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				auto int_ptr = static_cast<calc::Int64*>(elem.get());
				return int_ptr->get_str() + L" ";
			}
		},
		{
			'X',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				auto var_ptr = static_cast<calc::Var*>(elem.get());
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
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				return L"( ";
			}
		},
		{
			')',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				return L") ";
			}
		},
		{
			'#',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				auto opt_ptr = static_cast<calc::Int64Opt*>(elem.get());
				std::wstringstream wss;
				wss << (wchar_t)opt_ptr->get_opt_type() << L" ";
				return wss.str();
			}
		},
		{
			'F',
			[](const std::unique_ptr<calc::Element>& elem) -> std::wstring {
				auto fptr = static_cast<calc::Int64_Format*>(elem.get());
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

	bool process_launch(const int opt_type) {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		if (rename_thread.joinable()) rename_thread.join();
		state_.store(STATE_ONGOING, std::memory_order_release);

		switch (opt_type)
		{
			case 0:
			{
				rename_thread = std::thread(std::bind(&ProcessThread::rename_thread_assist_expr, this));
				break;
			}

			case 1:
			{
				rename_thread = std::thread(std::bind(&ProcessThread::rename_thread_assist_auto, this));
				break;
			}

			default:
			{
				break;
			}
		}

		return true;
	}

	ProcessThread() {
		msg_box_.store(false, std::memory_order_release);
		state_.store(STATE_READY, std::memory_order_release);
	}

	virtual ~ProcessThread() {
		if (rename_thread.joinable()) {
			rename_thread.join();
		}
	}

	bool reset_selected_file() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			auto lck = vec_filepath_cache.AcquireLock();
			lck->clear();
		}

		return true;
	}

	bool push_filepath(const std::wstring& filepath) {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			auto lck = vec_filepath_cache.AcquireLock();
			lck->emplace_back(filepath);
		}

		return true;
	}

	bool reset_input_expr_ptr() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			auto lck = input_expr.AcquireLock();
			lck->clear();
		}

		return true;
	}

	bool pop_expr_ptr() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			auto lck = input_expr.AcquireLock();
			if (!lck->empty()) {
				lck->pop_back();
			}
		}

		return true;
	}

	template <typename PtrType, typename... Args>
	bool push_expr(Args&&... args) {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return false;

		{
			auto lck = input_expr.AcquireLock();
			lck->emplace_back(std::make_unique<PtrType>(std::forward<Args>(args)...));
		}

		return true;
	}

	std::wstring get_res_wstr() {
		auto lck = res_wstr.AcquireLock();
		return *lck;
	}

	int get_state() {
		return state_.load(std::memory_order_acquire);
	}

	int get_and_clear_msg_box() {
		return msg_box_.exchange(false, std::memory_order_acq_rel);
	}

	std::wstring get_expression_str() {
		if (state_.load(std::memory_order_acquire) == STATE_ONGOING) return L"";

		auto lck = input_expr.AcquireLock();
		if (lck->empty()) {
			return L"";
		}

		std::wstringstream rewss;
		for (const auto& elem : *lck) {
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