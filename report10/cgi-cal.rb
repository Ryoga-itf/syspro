#!/usr/local3/coins/linux/bin/ruby
# -*- coding: utf-8 -*-
require "cgi"

def main()
  @cgi = CGI.new()
  print_header()
  print_content()
  exit(0)
end

def print_header()
  printf("Content-Type: text/html\n")
  printf("\n")
end

def print_content()
  printf("<HTML><HEAD></HEAD><BODY><PRE>\n")
  $stdout.flush()
  
  valid = true

  year_s = @cgi["year"]
  year = 0
  case(year_s)
  when ""
    printf("Parameter 'year' is required but empty.\n");
    valid = false
  else
    year = year_s.to_i()
    if year <= 0 || year >= 10000 then
      printf("Invalid value.\n");
      valid = false
    end
  end

  month_s = @cgi["month"]
  month = 0
  case(month_s)
  when ""
    printf("Parameter 'month' is required but empty.\n");
    valid = false
  else
    month = month_s.to_i()
    if month <= 0 || month > 12 then
      printf("Invalid value.\n");
      valid = false
    end
  end

  if valid then
    child_pid = fork do
      exec("/usr/bin/cal", sprintf("%d", month), sprintf("%d", year))
    end
    Process.waitpid(child_pid)
  end

  printf("</PRE></BODY></HTML>\n")
end

main()
