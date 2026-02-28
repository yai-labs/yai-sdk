# Contributing

## Scope

This repo focuses on the public CLI surface and contract alignment.
Change C/H/tests only when explicitly requested by the task.

## Commit Convention

- Small, descriptive commits
- One clear goal per commit
- No unrelated mixed changes

## Pull Request Rules

- Explain what changed and why
- State compatibility impact (`COMPATIBILITY.md`)
- Avoid giant PRs; prefer incremental PRs

## Minimum Checks

Before opening a PR:
- local build (`make`)
- verify the working tree does not include unwanted artifacts
- verify alignment with `deps/yai-law`
