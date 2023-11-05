# Git
## Worktrees
The `.git` inside a worktree file contains a path to the gitdir of a worktree. `gitdir: /mnt/work/git/essence.git/worktrees/master`. This doesn't work when repository is shared between different operating systems i.e Windows and Linux.
* Replace this with a relative path like: `gitdir: ../worktrees/master`

A bare repository doesn't set up tracking of remote branches, therefore `git pull` won't fetch a new branch from remote. [Workarounds to Git worktree using bare repository and cannot fetch remote branches](https://morgan.cugerone.com/blog/workarounds-to-git-worktree-using-bare-repository-and-cannot-fetch-remote-branches/)
* Execute after cloning the repo `git config remote.origin.fetch "+refs/heads/*:refs/remotes/origin/*"`
