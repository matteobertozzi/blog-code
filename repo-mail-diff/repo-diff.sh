#!/bin/bash
# http://www.pixelbeat.org/scripts/ansi2html.sh

diff_dir="~/.repo-diffs"
repo_url="~/src/"

ansi2html="/usr/bin/ansi2html.sh"

_check_for_changes() {
    if [ `ls -l $1.diff | awk '{print $5}'` -lt 1  ]; then
        rm -f $1.html
        rm -f $1.diff
    fi
}

git_diff() {
    cd $repo_url/$1

    git_repo_url=`git remote show origin | grep "Fetch URL" | cut -d ' ' -f 5-`
    echo "GIT Diff $1 ($2) - $git_repo_url"

    git fetch
    git diff --color HEAD origin/HEAD | $ansi2html > $diff_dir/$2.html
    git diff HEAD origin/HEAD > $diff_dir/$2.diff
    git merge origin/HEAD

    _check_for_changes $diff_dir/$2
}

hg_diff() {
    cd $repo_url/$1

    hg_repo_url=`hg showconfig | grep paths\.default | cut -d '=' -f 2-`
    echo "HG Diff $1 ($2) - $hg_repo_url"

    hg incoming --patch --git | $ansi2html > $diff_dir/$2.html
    hg incoming --patch --git  > $diff_dir/$2.diff
    hg pull -u

    _check_for_changes $diff_dir/$2
}

svn_diff() {
    cd $repo_url/$1

    svn_repo_url=`svn info | grep URL | cut -d ' ' -f 2-`
    svn_repo_rev=`svn info | grep "Last Changed Rev" | cut -d ' ' -f 4-`
    echo "SVN Diff $1 ($2) - $svn_repo_url"

    svn di $svn_repo_url -r$svn_repo_rev | $ansi2html > $diff_dir/$2.html
    svn di $svn_repo_url -r$svn_repo_rev > $diff_dir/$2.diff
    svn up

    _check_for_changes $diff_dir/$2
}


# Add Your Repos
# xxx_diff <repo path> <output file name>
#
# Example:
# $ mkdir ~/src/
# git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
# svn co https://svn.apache.org/repos/asf/lucene/dev/trunk/lucene
# hg clone http://hg.openjdk.java.net/jdk7/hotspot/jdk/

# git_diff linux-2.6 linux-kernel
# svn_diff lucene lucene
# hg_diff jdk hotspot-jdk7

