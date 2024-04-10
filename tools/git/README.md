# Git
## Worktrees
The `.git` inside a worktree file contains a path to the gitdir of a worktree. `gitdir: /mnt/work/git/essence.git/worktrees/master`. This doesn't work when repository is shared between different operating systems i.e Windows and Linux.
* Replace this with a relative path like: `gitdir: ../worktrees/master`

A bare repository doesn't set up tracking of remote branches, therefore `git pull` won't fetch a new branch from remote. [Workarounds to Git worktree using bare repository and cannot fetch remote branches](https://morgan.cugerone.com/blog/workarounds-to-git-worktree-using-bare-repository-and-cannot-fetch-remote-branches/)
* Execute after cloning the repo `git config remote.origin.fetch "+refs/heads/*:refs/remotes/origin/*"`

## Subtrees
If a subtree was added with a `--squash` then pulling to a subtree also needs to have `--squash` set.
* Add subtree to repository: `git subtree --prefix .base add --squash git@github.com:jan-kelemen/dotfiles.git cb1e342944645965dc35e156f24279bc6e127b24`
* Merge changes from git subtree: `git subtree --prefix .base pull --squash git@github.com:jan-kelemen/dotfiles.git cb1e342944645965dc35e156f24279bc6e127b24`

## Useful commands
List unmerged branches
```
git branch --no-merged master
```

Cumulative changes by author:
```
git log --no-merges --shortstat --pretty="%cE" | sed 's/\(.*\)@.*/\L\1/' | grep -v "^$" | awk 'BEGIN { line=""; } !/^ / { if (line=="" || !match(line, $0)) {line = $0 "," line }} /^ / { print line " # " $0; line=""}' | sort | sed -E 's/# //;s/ files? changed,//;s/([0-9]+) ([0-9]+ deletion)/\1 0 insertions\(+\), \2/;s/\(\+\)$/\(\+\), 0 deletions\(-\)/;s/insertions?\(\+\), //;s/ deletions?\(-\)//' | awk 'BEGIN {name=""; files=0; insertions=0; deletions=0;} {if ($1 != name && name != "") { print name ": " files " files changed, " insertions " insertions(+), " deletions " deletions(-), " insertions-deletions " net"; files=0; insertions=0; deletions=0; name=$1; } name=$1; files+=$2; insertions+=$3; deletions+=$4} END {print name ": " files " files changed, " insertions " insertions(+), " deletions " deletions(-), " insertions-deletions " net";}'
```
