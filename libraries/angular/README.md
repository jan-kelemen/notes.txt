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
## Creating a component
> ng generate component --inline-template false --inline-style false modules/ingredients/components/add-ingredient

## Routing
### Add routing to an application without routing
[Routing guide](https://angular.io/guide/router)
> ng generate module app-routing --flat --module=app
* Import the `AppRoutingModule` in `AppModule`
* Register the necessary routes
```
import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

const routes: Routes = [
  { path: 'users', component: UsersComponent },
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
```
### Route to a lazy loaded module
[Lazy loading NgModules](https://angular.io/guide/lazy-loading-ngmodules)
* Create the module
> ng generate module --route recipes --module app.module modules/recipes
* Route the module in `AppRoutingModule`
```
  {
    path: 'recipes',
    loadChildren: () => import('./modules/recipes/recipes.module').then(m => m.RecipesModule)
  },
```
### Redirecting
* Redirect empty urls `{ path: '', redirectTo: 'ingredients', pathMatch: 'full' }`
* Redirect unmatched urls `{ path: '**', component: PageNotFoundComponent }`
