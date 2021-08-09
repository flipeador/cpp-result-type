# Result — C++20 using modules

## Overview

`Result<ok_t, err_t>` is a template type that can be used to return and propage errors.
It can be used to replace exceptions in context where they are not allowed or too slow to be used.
`Result<ok_t, err_t>` is an algebraic data type of `Ok(ok_t)` that represents success and `Err(err_t)` representing an error.

Design of this class has been mainly inspired by Rust's [std::result](https://doc.rust-lang.org/std/result/)

```cpp
import <iostream>;
import <string>;
import result;

using namespace std;

struct Request {
    Request() { cout << "Request()" << endl; }
    ~Request() { cout << "~Request()" << endl; }
};

struct Error {
    enum class Kind : short {
        Timeout,
        Invalid,
        TooLong
    };

    Error(Kind kind, string msg)
        : kind(kind), msg(msg)
    { cout << "Error()" << endl; }

    ~Error() { cout << "~Error()" << endl; }

    Kind kind;
    string msg;
};

Result<shared_ptr<Request>, shared_ptr<Error>> ParseRequest(const string& payload, size_t maxsize = 512)
{
    if (payload.size() > maxsize)
        return Err(make_shared<Error>(Error::Kind::TooLong, "request exceeded maximum allowed size"));

    auto request = make_shared<Request>();
    return Ok(request);
}

int main()
{
    auto request = ParseRequest("payload", 1);

    if (request)
        cout << "Success!" << endl;
    else {
        auto error = request.GetErr();

        string kind("Unknown");
        switch (error->kind)
        {
        case Error::Kind::Timeout: kind = "Timeout"; break;
        case Error::Kind::Invalid: kind = "Invalid"; break;
        case Error::Kind::TooLong: kind = "TooLong"; break;
        }

        cout << "Failure!" << endl;
        cout << "Kind: " << kind << endl;
        cout << "Message: " << error->msg << endl;
    }

    return 0;
}
```

To return a successfull `Result`, use the `Ok()` function. To return an error one, use the `Err()` function.

---

## TryGet, GetOk, GetErr and Expect

To extract the value from a `Result<ok_t, err_t>` type, you can use the `Expect()` function that will yield the value of an `Ok(ok_t)` or terminate the program with an error message passed as a parameter.

```cpp
Result<int, int> result = Ok(3);
auto ret = result.Expect("failed to retrieve the value");  // can be None
assert(ret == 3);  // true
```

`GetOk()` can also be used to extract the value of a `Result`, yielding the value of an `Ok(ok_t)` value or terminating the program otherwise:

```cpp
Result<int, int> result = Ok(3);
auto ret = result.GetOk();  // cannot be None
assert(ret == 3);  // true
```

Instead a terminating the program, `TryGet` can be used to return a default value for an `Err(err_t)` Result:

```cpp
Result<int, int> result = Err(0);
std::cout << result.TryGet(7) << std::endl;  // 7
```
