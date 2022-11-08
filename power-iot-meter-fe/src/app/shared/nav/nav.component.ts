import { Component, OnInit, Output, EventEmitter } from '@angular/core';


@Component({
  selector: 'app-nav',
  templateUrl: './nav.component.html',
  styleUrls: ['./nav.component.scss']
})
export class NavComponent implements OnInit {
  
  @Output()
  LogOutClick = new EventEmitter<any>();

  constructor() { }

  ngOnInit(): void {
  }

  

}
