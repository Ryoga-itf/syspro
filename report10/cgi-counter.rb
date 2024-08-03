#!/usr/local3/coins/linux/bin/ruby
# -*- coding: utf-8 -*-
require "cgi"

DATA_FILENAME = "file-counter-value.data"
$comname = $0

def main()
  @cgi = CGI.new()
  print_header()
  counter =  handle_counter()
  print_content(counter)
  exit( 0 )
end

def print_header()
  printf("Content-Type: text/html\n")
  printf("\n")
end

def handle_counter()
  counter = counter_load()
  op = @cgi["op"]
  val = @cgi["val"].to_i()

  case(op)
  when "inc"
    counter += 1
    counter_save(counter)
  when "set"
    counter = val
    counter_save(counter)
  else
  end

  return counter
end

def print_content(counter)
  printf("<HTML><HEAD></HEAD><BODY>\n")
  $stdout.flush()
  printf("%d\n", counter)
  printf("</BODY></HTML>\n")
end

def counter_save(counter)
  data = [counter].pack("i") # signed int
  IO.binwrite(DATA_FILENAME, data)
end

def counter_load()
  counter = 0
  begin
    data = IO.read(DATA_FILENAME, 4)
    counter = data.unpack("i")[0]
  rescue Errno::ENOENT
    # nop
  end
    return counter
end

main()
