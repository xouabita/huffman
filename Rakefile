require 'fileutils'

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
  if File.exist?('./huff.out') then FileUtils.rm './huff.out' end
  if File.exist?('./results/') then FileUtils.rm_rf './results/' end
end
