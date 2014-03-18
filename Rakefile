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
  del 'results'
  FileUtils.mkdir 'results'
  sh 'touch ./results/no_write.in'
  sh 'chmod -wx+r ./results/no_write.in'
  in_files = Dir.glob('tests/*.huf')
  out_files = Dir.glob('tests/*.in')

  in_files.each do |inp|
    out = inp.sub('tests/','results/').sub('.huf','.in')
    sh "./huff.out #{inp} #{out}"
  end

  puts "--------------------"

  res_files = Dir.glob('results/*.in')
  res_files.each do |res|
    out_files.each do |o|
      if o.sub('tests/', "") == res.sub('results/', "") then
        if (sh "diff #{o} #{res}") then puts 'OK!' end
      end
    end
  end

end

desc "Clean the dir"
task :clean do
  del 'results'
  del Dir.glob('test.*')
  del 'huff.out'
end
