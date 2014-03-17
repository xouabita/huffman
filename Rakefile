require 'fileutils'

def del arr
  if arr.is_a? String then arr = [arr] end
  arr.each do |str|
    if File.exist?(str) then
      if File.directory?(str) then FileUtils.rm_rf str
      else FileUtils.rm str end
    end
  end
end


task :default => 'build'

desc "Build the proj"
task :build do
  sh "g++ huff.cpp -pedantic -Wall -o huff.out"
end

desc "Make the test"
task :test => :build do
  FileUtils.mkdir 'results'
  (0..5).each do |n|
    input  = "./tests/test#{n}.huf"
    output = "./results/test#{n}.in"
    puts "Running test ##{n}"
    sh "./huff.out #{input} #{output}"
    dif_cmd = "diff ./tests/test#{n}.in #{output}"
    puts(sh dif_cmd)
  end
end

desc "Clean the dir"
task :clean do
  del 'results'
  del Dir.glob('test.*')
  del 'huff.out'
end
