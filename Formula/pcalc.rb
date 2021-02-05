# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.8.tar.gz"
  sha256 "7be8f374b3e9d1e27c496d3cfe168e7053f5778449ac44868b9c697995541ad3"
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
