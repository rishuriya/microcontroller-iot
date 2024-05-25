import 'dart:convert';
import 'dart:ui';

import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';

class Home extends StatefulWidget {
  const Home({super.key});

  @override
  State<Home> createState() => _HomeState();
}

class _HomeState extends State<Home> {
  List<Appliance> _rooms = [];
  DatabaseReference rtdb = FirebaseDatabase.instanceFor(
          app: Firebase.app(),
          databaseURL:
              'https://microcontroller-iot-default-rtdb.asia-southeast1.firebasedatabase.app/').ref('test');

  Stream<DatabaseEvent>? _stream;
  initState() {
    super.initState();
    _stream = rtdb.onValue;
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: DecoratedBox(
        decoration: BoxDecoration(
          image: DecorationImage(
            image: const AssetImage('assets/image/background.jpeg'),
            fit: BoxFit.cover,
          ),
        ),
        child: Container(
          color: Colors.black.withOpacity(0.3),
          height: double.infinity,
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 20.0, vertical: 60.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Container(
                  alignment: Alignment.centerRight,
                  child: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                'Good Morning',
                                style: TextStyle(
                                    color: Colors.white,
                                    fontWeight: FontWeight.bold,
                                    fontSize: 30.0),
                              ),
                              Text(
                                'Welcome Back!',
                                style: TextStyle(
                                    color: Colors.white,
                                    fontWeight: FontWeight.bold,
                                    fontSize: 18.0),
                              ),
                            ]),
                        Column(
                            crossAxisAlignment: CrossAxisAlignment.end,
                            children: [
                              ClipOval(
                                child: BackdropFilter(
                                  filter: ImageFilter.blur(
                                      sigmaX: 10.0, sigmaY: 10.0),
                                  child: Container(
                                    width: 40.0,
                                    height: 40.0,
                                    decoration: BoxDecoration(
                                        borderRadius:
                                            BorderRadius.circular(20.0),
                                        color: Colors.grey.shade200
                                            .withOpacity(0.1)),
                                    child: Icon(
                                      Icons.menu,
                                      color: Colors.white,
                                    ),
                                  ),
                                ),
                              ),
                              ClipOval(
                                child: BackdropFilter(
                                  filter: ImageFilter.blur(
                                      sigmaX: 10.0, sigmaY: 10.0),
                                  child: Container(
                                    width: 40.0,
                                    height: 40.0,
                                    decoration: BoxDecoration(
                                        borderRadius:
                                            BorderRadius.circular(20.0),
                                        color: Colors.grey.shade200
                                            .withOpacity(0.1)),
                                    child: Icon(
                                      Icons.notifications_none,
                                      color: Colors.white,
                                    ),
                                  ),
                                ),
                              ),
                            ])
                      ])),
              Container(
                  child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: <Widget>[
                  Text(
                    'OUR HOME',
                    style: TextStyle(
                        color: Colors.white,
                        fontSize: 48.0,
                        wordSpacing: 2.0,
                        fontWeight: FontWeight.bold),
                  ),
                  Center(
                    child: StreamBuilder<DatabaseEvent>(
                        stream: _stream,
                        builder: (context, snapshot) {
                          if (snapshot.hasData) {
                            var data = snapshot.data!.snapshot.value as Map;
                            print(data);
// Create a list of Appliance objects
                            _rooms = data.entries.map((entry) {
                              int id = int.parse(entry.key);
                              String name = entry.value['name'];
                              String state = entry.value['state'];
                              bool status = state == 'HIGH';
                              return Appliance(id, name, state, status);
                            }).toList();
                          }
                          return Wrap(
                            alignment: WrapAlignment.end,
                            spacing: 10.0,
                            runSpacing: 10.0,
                            children: [
                              frostedRect(
                                  MediaQuery.of(context).size.width / 2 + 30,
                                  200.0,
                                  'Bedroom',
                                  'Google Nest \nHub Max',
                                  Icons.speaker),
                              frostedRect(
                                  MediaQuery.of(context).size.width / 2 - 88,
                                  200.0,
                                  'Balcony',
                                  'Philips smart\nLed Bulb',
                                  Icons.light),
                              frostedRect(
                                  MediaQuery.of(context).size.width / 2 - 28,
                                  250.0,
                                  'Kitchen',
                                  'Godgej\nAir Exhaust',
                                  Icons.free_breakfast),
                              frostedRect(
                                  MediaQuery.of(context).size.width / 2 - 28,
                                  250.0,
                                  'Living',
                                  'Samsung Smart\nLed TV',
                                  Icons.tv),
                            ],
                          );
                        }),
                  )
                ],
              ))
            ],
          ),
        ),
      ),
    );
  }

  Widget frostedRect(double width, double height, String title, String subtitle,
      IconData icon) {
    return ClipRect(
        child: BackdropFilter(
            filter: ImageFilter.blur(sigmaX: 10.0, sigmaY: 10.0),
            child: Container(
              width: width,
              height: height,
              padding: EdgeInsets.only(
                  left: 10.0, right: 0.0, top: 10.0, bottom: 10.0),
              decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.2),
                  borderRadius: BorderRadius.circular(20.0)),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Row(
                      mainAxisSize: MainAxisSize.max,
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Icon(icon, color: Colors.white, size: 20.0),
                        SizedBox(
                            child: Transform.rotate(
                          angle: -3.14 / 2, // Rotate the switch by -90 degrees
                          child: Switch(
                            value: _rooms.where((element) => element.name == title)?.firstOrNull?.status ?? false,
                            onChanged: (bool value) {
                              setState(() {
                                if (value) {
                                  rtdb.child('${_rooms.where((element) => element.name == title).first.id}').update({'state': 'HIGH'});
                                } else {
                                  rtdb.child('${_rooms.where((element) => element.name == title).first.id}').update({'state': 'LOW'});
                                }
                              });
                            },
                            activeColor: Colors.blue,
                            activeTrackColor: Colors.blue.shade200,
                          ),
                        ))
                      ]),
                  Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          title,
                          style: TextStyle(
                              color: Colors.white,
                              fontSize: 24.0,
                              fontWeight: FontWeight.bold),
                        ),
                        Text(
                          subtitle,
                          style: TextStyle(
                              color: Colors.white.withOpacity(0.5),
                              fontSize: 15.0),
                        ),
                      ])
                ],
              ),
            )));
  }
}

class Appliance {
  int? id;
  String? name;
  String? state;
  bool? status;
  Appliance(this.id,this.name, this.state, this.status);
}
