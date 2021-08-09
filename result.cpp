/*
   Mathieu Stefani, 03 mai 2016

   This header provides a Result type that can be used to replace exceptions in code that has to handle error.
   Result<ok_t, err_t> can be used to return and propagate an error to the caller.
   Result<ok_t, err_t> is an algebraic data type that can either Ok(ok_t) to represent success or Err(err_t) to represent an error.

   https://github.com/flipeador, 20210809
*/
module;  // global

#include <iostream>

export module result;

struct None__ { };

template <typename ok_t>
struct Success
{
	Success(const ok_t& ok)
		: value(ok)
	{ }

	Success(ok_t&& ok)
		: value(std::move(ok))
	{ }

	ok_t value;
};

template <typename err_t>
struct Failure
{
	Failure(const err_t& err)
		: value(err)
	{ }

	Failure(err_t&& err)
		: value(std::move(err))
	{ }

	err_t value;
};

export
{
	typedef None__* None;

	/// <summary>
	/// Represents success without a value (void).
	/// </summary>
	inline Success<None> Ok()
	{
		return Success<None>(nullptr);
	}

	/// <summary>
	/// Represents success containing a value.
	/// </summary>
	template<typename ok_t, typename C = typename std::decay_t<ok_t>>
	Success<C> Ok(ok_t&& value) {
		return Success<C>(std::forward<ok_t>(value));
	}

	/// <summary>
	/// Represents an error containing an error value.
	/// </summary>
	template<typename err_t, typename C = typename std::decay_t<err_t>>
	Failure<C> Err(err_t&& value) {
		return Failure<C>(std::forward<err_t>(value));
	}

	/// <summary>
	/// Error handling with the Result class, similar to Rust's std::result.
	/// </summary>
	template <typename ok_t, typename err_t>
	class Result
	{
		// The error value cannot be None.
		static_assert(!std::is_same_v<err_t, void>, "'None error type is not allowed");

		static constexpr size_t _size = sizeof(ok_t) > sizeof(err_t) ? sizeof(ok_t) : sizeof(err_t);
		static constexpr size_t _align = sizeof(ok_t) > sizeof(err_t) ? alignof(ok_t) : alignof(err_t);

	public:
		Result(const Success<ok_t>& ok)
			: m_success(true)
		{
			new (&m_storage) ok_t(std::move(ok.value));
		}

		Result(const Failure<err_t>& err)
			: m_success(false)
		{
			new (&m_storage) err_t(std::move(err.value));
		}

		Result(const Result& result)
			: m_success(result.m_success)
		{
			if (m_success)
				new (&m_storage) ok_t(std::forward<const ok_t>(result.Ok()));
			else
				new (&m_storage) err_t(std::forward<const err_t>(result.Err()));
		}

		Result(Result&& result)
			: m_success(result.m_success)
		{
			if (m_success)
				new (&m_storage) ok_t(std::forward<ok_t>(result.Ok()));
			else
				new (&m_storage) err_t(std::forward<err_t>(result.Err()));
			result.~Result();
		}

		~Result()
		{
			if (m_success)
				Ok().~ok_t();
			else
				Err().~err_t();
		}

		template <class T = ok_t>
		typename std::enable_if_t<!std::is_same_v<T, None>, T>
		TryGet(const T& defval) const
		{
			if (m_success)
				return Ok();
			return defval;
		}

		/// <summary>
		/// Gets the success value. This function cannot be called when the value is None.
		/// If the result is an error, the program terminates with an error message.
		/// </summary>
		template <class T = ok_t>
		typename std::enable_if_t<!std::is_same_v<T, None>, T>
		GetOk() const
		{
			if (!m_success)
				ErrorTerminate("Attempting to Result::GetOk an error Result.");
			return Ok();
		}

		/// <summary>
		/// Gets the error value.
		/// If the result is not an error, the program terminates with an error message.
		/// </summary>
		err_t GetErr() const
		{
			if (m_success)
				ErrorTerminate("Attempting to Result::GetErr an ok Result.");
			return Err();
		}

		/// <summary>
		/// Gets the success value, can be None.
		/// If the result is an error, the program terminates with the specified error message.
		/// </summary>
		ok_t Expect(const char* msg) const
		{
			if (!m_success)
				ErrorTerminate(msg);
			return std::is_same_v<ok_t, None> ? nullptr : Ok();
		}

		/// <summary>
		/// Determine if the result is successful.
		/// </summary>
		operator bool()
		{
			return m_success;
		}

	private:
		ok_t& Ok() { return *reinterpret_cast<ok_t*>(&m_storage); }
		const ok_t& Ok() const { return *reinterpret_cast<const ok_t*>(&m_storage); }
		err_t& Err() { return *reinterpret_cast<err_t*>(&m_storage); }
		const err_t& Err() const { return *reinterpret_cast<const err_t*>(&m_storage); }

		void ErrorTerminate(const char* str) const
		{
			std::cerr << str << std::endl;
			std::terminate();
		}

		bool m_success;
		std::aligned_storage_t<_size, _align> m_storage;
	};
}
