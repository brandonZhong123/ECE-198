## Common Commands in this Workflow

1. `git pull`
    - assumed to be origin (branch youre working on)
    - you can change `main` into any branch that youd like to pull
        - e.g. `git pull origin brandonzhong`
    - you can also do `git pull --ff-only`, its helpful since it only pulls if Git can fast forward ur branch without having a conflict so ur not in the middle of a predicament

2. `git push`
    - assumed to be origin (branch youre working on) as well
    - can be specified like: `git push origin brandonzhong`
    - to set upstream (let remote know what remote branch you want this local branch to be associated with to since it's your first push on a new branch): `git push --set-upstream origin (branchname)`

3. `git add (filename1) (filename2) ...`, `git commit`
    - those two go hand in hand
    - you must have files added to be tracked and staged in order to commit them
    - if you dont have files added / modified, commits dont count
    - specifically for git commit, you may add a message like `git commit -m "Overhauled UI design"` with the shorthand -m

4. `git checkout -b (branchname)`
    - make your own branch, e.g. `git checkout -b alvernwang`

5. `git checkout (branchname)`
    - switch to a specific branch
    - make sure you `git stash` beforehand

6. `git stash`
    - puts your uncommited edits to a stash
    - can also push to a stash like `git stash push`

7. `git merge`
    - merges your current code state with whatever branch you chose to merge from
    - combines two git commit nodes into one like a subway table
    - can introduce merge conflicts where upstream code disagrees with the code you just wrote and IDE has no idea which one to actually take

8. `git rebase`
    - replays your current commit on top of all previous commits (could be by others) and keeps a clean git history graph
    - can still have conflicts that you need to resolve

After you push something from your local branch to remote, you can open GitHub and see your most recent push on the main page. You can open a PR there. If GitHub warns you about merge conflicts, resolve it locally and then recommit + repush. (include --force-with-lease in your push cmd if it is for a rebase)

More basic commands that arent complicated and u can just use:
- `git status` --> what branch ur working on, tracked files
- `git branch` --> what branches you have
- `git log --oneline` --> shows shortened commit histories from current head 

## Some more useful things to do / know in a collaborative repo

There are always going to be merge and rebase conflicts here and there when ur working
on branches and pushing to main. Every time when you check out a new branch, make sure:
- you are checking out the desired branch (99.99999% of the times it should be `master` or `main`, not a personal random branch)
- `git fetch` and `git pull` for your `main` branch first to ensure your local main is up to date so that you dont have to deal with random conflicts later
- when conflicts arise during a `git merge` or `git rebase`, resolve the conflicts in your IDE (not sure if arduino has this), and do `git merge --continue` or `git rebase --continue`
