#pragma once
#include <functional>
#include <map>

template <typename TestEnum, TestEnum MaxValue> class TestManager {
public:
  using TestFunction = std::function<bool()>;
  struct TestInfo {
    TestFunction func;
    bool softFail;
  };

  void AddTest(TestEnum id, TestFunction func, bool softFail = false) {
    tests_[id] = {func, softFail};
  }

  void SetAfterTestHook(std::function<void(bool, TestEnum)> hook) {
    afterHook_ = hook;
  }

  bool Start(TestEnum first, TestEnum last) {
    bool success = true;
    for (int i = static_cast<int>(first); i <= static_cast<int>(last); ++i) {
      auto it = tests_.find(static_cast<TestEnum>(i));
      if (it == tests_.end())
        continue;
      bool result = it->second.func();
      if (afterHook_)
        afterHook_(result, static_cast<TestEnum>(i));
      if (!result && !it->second.softFail)
        success = false;
    }
    return success;
  }

private:
  std::map<TestEnum, TestInfo> tests_;
  std::function<void(bool, TestEnum)> afterHook_;
};
