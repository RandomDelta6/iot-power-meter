import { Subscription } from 'rxjs';
import { AngularFireAuth } from '@angular/fire/compat/auth';
import { Component, OnInit, OnDestroy } from '@angular/core';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import Measurements from 'src/app/dtos/measurement.dto';

const TOPIC = '/users';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss']
})
export class DashboardComponent implements OnInit, OnDestroy {
  precision: number = 3;
  authState?: Subscription;
  user: any;
  topicSub?: Subscription;
  dataSub?: Subscription;
  deviceTopic?: string;
  constructor(
    private rtdb: AngularFireDatabase,
    private auth: AngularFireAuth
  ) {

  }

  ngOnInit(): void {
    //check login, if so, subscribe to DB
    this.authState = this.auth.authState.subscribe(user => {
      this.user = user;
      if (user) {
        this.topicSub = this.rtdb
          .object(TOPIC + '/' + user.uid)
          .valueChanges()
          .subscribe(topic => {
            this.deviceTopic = topic as string;
            this.dataSub = this.rtdb
              .object(this.deviceTopic)
              .valueChanges()
              .subscribe(data => {
                this.dataObj = data as Measurements;

              });
          });
      }
    });

  }

  ngOnDestroy(): void {
    //unsubscribe from DB
    this.authState?.unsubscribe;
    this.topicSub?.unsubscribe;
  }

  dataObj: Measurements = {
    current_rms: 0,
    active_power: 0,
    voltage_rms: 0,
    total_energy: 0,
    frequency: 0,
    power_factor: 1,
    timestamp: '0000-00-00 00:00',
    power_outages: {
      "0000-00-00 00:01": "0000-00-00 00:00",
      "0000-00-00 00:02": "0000-00-00 00:00",
      "0000-00-00 00:03": "0000-00-00 00:00",
      "0000-00-00 00:04": "0000-00-00 00:00",
    }
  }
  billRate: number = 6.00;
  getBill(): number {
    return this.dataObj.total_energy * this.billRate;
  }

}
