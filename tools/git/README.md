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
