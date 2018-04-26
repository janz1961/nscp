import { Component, OnInit } from '@angular/core';
import {TranslateService} from '@ngx-translate/core';

@Component({
  selector: 'app-nav',
  templateUrl: './nav.component.html',
  styleUrls: ['./nav.component.css']
})
export class NavComponent implements OnInit {

  appConfig: any;
  menuItems: any[];
  progressBarMode: string;
  currentLang: string;

  constructor(private translateService: TranslateService) { }

  ngOnInit() {
  }

  changeLanguage(language: string): void {
    this.translateService.use(language).subscribe(() => {
      this.loadMenus();
    });
  }

  private loadMenus(): void {
    this.translateService.get(['home', 'heroesList'], {}).subscribe((texts: any) => {
      this.menuItems = [
        {link: '/', name: texts['home']}
        //{link: '/' + AppConfig.routes.heroes, name: texts['heroesList']}
      ];
    });
  }

}
