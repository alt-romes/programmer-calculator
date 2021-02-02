1) Set a tag for the version
```
git tag v(a.b)

git push --tags
```

2) Edit the new tag on GitHub to make it a release with title and description

3) Calculate and copy the sha256sum for the newly released version from the github generated tarball: https://github.com/alt-romes/programmer-calculator/archive/va.b.tar.gz
```
curl -L https://github.com/alt-romes/programmer-calculator/archive/va.b.tar.gz > va.b

sha256sum va.b
```

4) Edit the `Formula/pcalc.rb` - this is just to mirror the actual Formula being used
    - change the URL to the new release tarball URL
    - change the sha sum to the one just calculated

5) Edit the `Forumla/pcalc.rb` in the repository `alt-romes/homebrew-pcalc` and push the changes - this is the actual Formula being used (it's easy to edit it directly in GitHub)
    - change the URL to the new release tarball URL
    - change the sha sum to the one just calculated

6) Test the updated brew formula by running
```
brew update

brew upgrade pcalc
```

7) Edit `Formula/PKGBUILD` - this is just a mirror of the actual package installer
    - change the version `pkgver` to `a.b`
    - update the sha sum to the one just calculated

8) Clone the AUR repository https://aur.archlinux.org/programmer-calculator.git (you must be a colaborator)

9) Edit `PKGBUILD`
    - change the version `pkgver` to `a.b`
    - update the sha sum to the one just calculated

10) Edit `.SRCINFO`
    - change the version `pkgver` to `a.b`
    - change in `source` references to previous version to the new version `a.b`
    - change the shasum

11) Push changes to the AUR repo

12) Test the AUR repo (howto?)
