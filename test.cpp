#include "NanoLog.cpp"

int main()
{
  // Ensure initialize is called once prior to logging.
  // This will create log files like /tmp/nanolog1.txt, /tmp/nanolog2.txt etc.
  // Log will roll to the next file after every 1MB.
  // This will initialize the guaranteed logger.
  myNanoLog::initialize(myNanoLog::GuaranteedLogger(), "/home/wlx/myNanoLog/", "myNanoLog", 1);
  
  // Or if you want to use the non guaranteed logger -
  // ring_buffer_size_mb - LogLines are pushed into a mpsc ring buffer whose size
  // is determined by this parameter. Since each LogLine is 256 bytes,
  // ring_buffer_size = ring_buffer_size_mb * 1024 * 1024 / 256
  // In this example ring_buffer_size_mb = 3.
  // nanolog::initialize(nanolog::NonGuaranteedLogger(3), "/tmp/", "nanolog", 1);
  
  for (int i = 0; i < 15; ++i)
  {
    LOG_INFO << "Sample NanoLog: " << i << " over";
  }
  
  // Change log level at run-time.
  myNanoLog::set_log_level(myNanoLog::LogLevel::CRIT);
  LOG_WARN << "HAHAHAHAHA not printed!!!";
  
  return 0;
}