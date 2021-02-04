# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.7.tar.gz"
  sha256 "989a548ececaa35ed18d452efb805524bfef55566bf73ad13d5085ac56a51057"
  license "GNU General Public License v3.0"

  depends_on "cmake" => :build
  depends_on "ncurses"

  def install
    system "make"
    bin.install "pcalc"
  end

  test do
    system "true"
  end
end
