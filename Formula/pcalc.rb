# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.6.tar.gz"
  sha256 "400693bfccf905e4def370125bb1d13f275a07954fbe14e9aaf4846e3ddf299f"
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
