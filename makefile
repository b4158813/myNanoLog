TEST_THREADCOUNT=test_different_thread_count.cpp
TEST_THREADCOUNT_TARGET=test_threadcount
TEST_VS_SPDLOG=myNanoLog_vs_spdlog.cpp
TEST_VS_SPDLOG_TARGER=test_vs
SPDLOG_INCLUDE=/home/wlx/spdlog/include

all:
	$(CXX) $(TEST_VS_SPDLOG) -std=c++11 -O3 -pthread -I $(SPDLOG_INCLUDE) -o $(TEST_VS_SPDLOG_TARGER)
	$(CXX) $(TEST_THREADCOUNT) -std=c++11 -O3 -pthread -o $(TEST_THREADCOUNT_TARGET)