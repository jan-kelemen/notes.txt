# Angular
## Setup Node.js and Angular CLI
* https://angular.io/guide/setup-local
* Install Node.js from [Nodesource](https://github.com/nodesource/distributions)
> curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash - && sudo apt install -y nodejs\
> sudo npm install -g @angular/cli

## Create a new Angular application
* Create a new workspace
  * [Strict mode](https://angular.io/guide/strict-mode)
  * Use CSS for styling, don't generate routing
> ng new --minimal --strict --routing false --style css --skip-git app
  * Delete redundant files from the template
> rm -rf app/.vscode && rm app/README.md
* Check if the application is running
  * `-o` opens up the default browser with the application running
> cd app && ng serve -o
