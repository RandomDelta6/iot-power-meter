import { Component, OnDestroy, OnInit } from '@angular/core';
import { AngularFireAuth } from '@angular/fire/compat/auth';
import { FirebaseUISignInFailure, FirebaseUISignInSuccessWithAuthResult } from 'firebaseui-angular';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-main',
  templateUrl: './main.component.html',
  styleUrls: ['./main.component.scss']
})
export class MainComponent implements OnInit, OnDestroy {
  isSignedIn: boolean = false;
  authStateSub?: Subscription;
  constructor(private auth: AngularFireAuth) { }

  ngOnInit(): void {
    this.authStateSub =
      this.auth.authState
        .subscribe(data => {
          if(data !== null) {
            this.isSignedIn = true;
          }
        });
  }

  ngOnDestroy(): void {

  }

  successCallback(signInSuccessData: FirebaseUISignInSuccessWithAuthResult) {
    console.log(signInSuccessData);
    this.isSignedIn = true;

  }

  errorCallback(errorData: FirebaseUISignInFailure) {
    console.log(errorData);

  }

  uiShownCallback() {

  }

  signOut(): void {
    this.auth.signOut();
    this.isSignedIn = false;

  }

}
