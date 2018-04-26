import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import {MatButtonModule, MatCheckboxModule} from '@angular/material';

import { AppComponent } from './app.component';
import {AppRoutingModule} from './app-routing.module';
import {SharedModule} from './shared/shared.module';

import {HTTP_INTERCEPTORS, HttpClient, HttpClientModule} from '@angular/common/http';
import {TranslateLoader, TranslateModule} from '@ngx-translate/core';
import {HttpLoaderFactory} from './app.translate.factory';

import {NavComponent} from './nav/nav.component';
import {FooterComponent} from './footer/footer.component';

import {environment} from '../environments/environment';
import { SearchBarComponent } from './search-bar/search-bar.component';

import { BrowserAnimationsModule } from '@angular/platform-browser/animations';

import 'hammerjs';

@NgModule({
  declarations: [
    AppComponent,
    NavComponent,
    FooterComponent,
    SearchBarComponent
  ],
  imports: [
    BrowserModule,
    MatButtonModule,
    MatCheckboxModule,
    HttpClientModule,
    TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: HttpLoaderFactory,
        deps: [HttpClient]
      }
    }),
    SharedModule.forRoot(),
    AppRoutingModule,
    BrowserAnimationsModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
