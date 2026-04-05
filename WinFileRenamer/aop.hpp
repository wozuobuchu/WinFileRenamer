#ifndef _AOP_HPP
#define _AOP_HPP

#pragma once

#include <utility>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include <stop_token>
#include <memory>

namespace aop {

template <typename _Tp>
class LockBox {
private:
	class LockProxy;
	friend class LockProxy;

	std::mutex mtx_;
	_Tp obj_;

public:
	class LockProxy {
	private:
		friend class LockBox;

		std::mutex* mtx_ptr_ = nullptr;
		_Tp* obj_ptr_ = nullptr;

		LockProxy() = delete;
		LockProxy(const LockProxy&) = delete;
		LockProxy& operator=(const LockProxy&) = delete;
		LockProxy& operator=(LockProxy&& other) = delete;

		LockProxy(LockBox<_Tp>* box_ptr_) noexcept {
			mtx_ptr_ = &box_ptr_->mtx_;
			obj_ptr_ = &box_ptr_->obj_;

			mtx_ptr_->lock();
		}

	public:
		LockProxy(LockProxy&& other) noexcept : mtx_ptr_(other.mtx_ptr_), obj_ptr_(other.obj_ptr_) {
			other.mtx_ptr_ = nullptr;
			other.obj_ptr_ = nullptr;
		}

		~LockProxy() {
			if (mtx_ptr_) {
				mtx_ptr_->unlock();
			}
		}

		_Tp* operator->() { return obj_ptr_; }

		const _Tp* operator->() const { return obj_ptr_; }

		_Tp& operator*() { return *obj_ptr_; }

		const _Tp& operator*() const { return *obj_ptr_; }
	};

	LockBox() = default;

	template <typename... Args>
	LockBox(Args&& ...args) : obj_(std::forward<Args>(args)...) {}

	LockBox(const LockBox&) = delete;
	LockBox(LockBox&&) = delete;
	LockBox& operator=(const LockBox&) = delete;
	LockBox& operator=(LockBox&&) = delete;

	[[nodiscard]] LockBox<_Tp>::LockProxy AcquireLock() {
		return LockProxy(this);
	}
};

template <typename _Tp>
using LockGuard = typename LockBox<_Tp>::LockProxy;

}

#endif // !_AOP_HPP