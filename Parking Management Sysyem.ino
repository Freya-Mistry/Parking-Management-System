#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <TimeLib.h>
#include <MemoryFree.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define sync 0    //Define initial sync state
#define main 1    //Define main state
#define SELECT 2  //Define SELECT state

int state = sync;  //Initial state is the sync state
unsigned long currentTime = 0;
unsigned long lastTime;
int count = 0;
bool selectPressed = false;
bool changePay = false;
int displayedVehicle = 0;
unsigned long selectTime;
bool selectDisplayed = false;
bool display = false;

char locationScroll[11];  //Array for splitting the parking location into characters for the SCROLL feature

//Structure for keeping all vehicle information
struct VehicleInfo {
  String RegNumber;
  char VehicleType;
  String PaymentStatus;
  String ParkingLocation;
  String EnterParkingTime;
  String PaidParkingTime;
};

//Global structure size
int StructSize = 14;
//Create structure
VehicleInfo VehicleInfo[14];

String CheckTime() {
  //Return the time in a suitable format
  //Time starts from 0000 every time the program is ran
  String hour1;
  String minute1;
  String time;
  hour1 = (String)(hour());
  minute1 = (String)(minute());
  if (hour() < 10) {
    hour1 = "0" + hour1;
  }
  if (minute() < 10) {
    minute1 = "0" + minute1;
  }
  time = hour1 + minute1;
  return time;
}

void CheckRegNumber(String VehicleRegNumber) {
  //Makes sure that the registration number is formatted correctly
  if ((isalpha(VehicleRegNumber[0]) != 0) && (isalpha(VehicleRegNumber[1]) != 0)) {
    Serial.println(F("DEBUG: Starts with 2 letters"));
  } else {
    Serial.println(F("ERROR: Does not start with 2 letters"));
  }
  if ((isdigit((int)VehicleRegNumber[2]) != 0) && (isdigit((int)VehicleRegNumber[3]) != 0)) {
    Serial.println(F("DEBUG: Followed by 2 numbers"));
  } else {
    Serial.println(F("ERROR: Is not followed by 2 numbers"));
  }
  if ((isalpha(VehicleRegNumber[4]) != 0) && (isalpha(VehicleRegNumber[5]) != 0) && (isalpha(VehicleRegNumber[6]) != 0)) {
    Serial.println(F("DEBUG: Ends with 3 letters"));
  } else {
    Serial.println(F("ERROR: Does not end with 3 letters"));
  }
}

bool CheckVehicleType(char Type) {
  //Makes sure that the vehicle type is a valid option
  if ((Type == 'C') || (Type == 'M') || (Type == 'V') || (Type == 'L') || (Type == 'B')) {
    Serial.println(F("DEBUG: Correct vehicle type"));
    return true;
  } else {
    Serial.println(F("ERROR: Not correct vehicle type"));
    return false;
  }
}

bool CheckParkingLocation(String ParkingLocation) {
  //Makes sure that the parking location is valid
  bool Valid = true;
  if ((ParkingLocation.length() > 0) && (ParkingLocation.length() <= 11)) {
    Serial.println(F("DEBUG: Between 1 and 11 characters"));
    for (int i = 0; i <= (ParkingLocation.length() - 1); i++) {
      if (isalnum(ParkingLocation[i]) == 0 && ParkingLocation[i] != '.') {
        Serial.println(F("ERROR: Location is not in correct form"));
        Valid == false;
        return false;
      }
    }
    if (Valid == true) {
      return true;
    }
  } else {
    Serial.println(F("ERROR: Not between 1 and 11 characters"));
    return false;
  }
}

void AddToList(String Reg, char Type, String ParkingLoc) {
  bool same = false;
  //Checks if vehicle is already in the structure
  for (int i = 0; i < StructSize; i++) {
    if (VehicleInfo[i].RegNumber == Reg) {
      Serial.println(F("DEBUG: Vehicle already there"));
      same = true;
      //Changing vehicle type if different
      ChangeVehicleType(Reg, Type);
      //Changing parking location if different
      ChangeParkingLocation(Reg, ParkingLoc);
    }
  }
  if ((same == false) && (count < StructSize)) {
    //Adds vehicle to structure
    VehicleInfo[count].RegNumber = Reg;
    VehicleInfo[count].VehicleType = Type;
    VehicleInfo[count].PaymentStatus = "NPD";
    VehicleInfo[count].ParkingLocation = ParkingLoc;
    VehicleInfo[count].ParkingLocation.trim();
    VehicleInfo[count].EnterParkingTime = CheckTime();
    // Serial.print("DEBUG: ");
    // Serial.print(VehicleInfo[count].RegNumber);
    // Serial.print(VehicleInfo[count].VehicleType);
    // Serial.print(VehicleInfo[count].PaymentStatus);
    // Serial.print(VehicleInfo[count].ParkingLocation);
    // Serial.println(VehicleInfo[count].EnterParkingTime);
    count = count + 1;
  } else {
    Serial.println(F("ERROR: Vehicle cannot be added as already there or not enough space"));
  }
  state = main;
}

void ChangePaymentStatus(String RegNumber, String Status) {
  //Changes the payment status
  bool found = false;
  if (Status == "PD") {
    Status = " PD";
  }
  for (int i = 0; i < StructSize; i++) {
    if (VehicleInfo[i].RegNumber == RegNumber) {
      found = true;
      if (VehicleInfo[i].PaymentStatus == Status) {
        Serial.println(F("ERROR: Cannot be modified as same payment status"));
        break;
      } else {
        if (Status == " PD") {
          VehicleInfo[i].PaymentStatus = Status;
          VehicleInfo[i].PaidParkingTime = CheckTime();
          Serial.println(F("DEBUG: Payment status changed from NPD to PD"));
          display = true;
          displayVehicleInfo(i);
          break;
        } else if (Status == "NPD") {
          changePay = true;
          //Storing vehicle type and location as it is going to be removed
          char type;
          type = VehicleInfo[i].VehicleType;
          String location;
          location = VehicleInfo[i].ParkingLocation;
          //Remove vehicle from struct as the entry time will change so it will no longer be in order
          RemovingVehicle(RegNumber);
          //Adding the vehicle back to the struct as it will now be at the end of the struct
          //This means that the struct is still in order of entry time
          AddToList(RegNumber, type, location);
          Serial.println(F("DEBUG: Payment status changed from PD to NPD"));
          display = true;
          displayVehicleInfo(i);
          changePay = false;
          break;
        } else {
          Serial.println(F("ERROR: Not valid option for payment status"));
        }
      }
    }
  }
  if (found == false) {
    Serial.println(F("ERROR: This vehicle has not been added"));
  }

  state = main;
}

void ChangeVehicleType(String registration, char type) {
  //Check if vehicle type can be changed, if it can, then changes it before returning to the main state
  bool found = false;
  for (int i = 0; i < StructSize; i++) {
    if (VehicleInfo[i].RegNumber == registration) {
      found = true;
      if (VehicleInfo[i].VehicleType == type) {
        Serial.println(F("ERROR: Cannot be modified as same type of vehicle"));
        break;
      } else if (VehicleInfo[i].PaymentStatus == "NPD") {
        Serial.println(F("ERROR: Cannot be modified as non payment status"));
        break;
      } else if (CheckVehicleType(type) == false) {
        break;
      } else {
        //If a valid vehicle type has been input, then changes type
        VehicleInfo[i].VehicleType = type;
        Serial.println(F("DEBUG: Vehicle Type changed"));
        display = true;
        break;
      }
    }
  }
  if (found == false) {
    Serial.println(F("ERROR: This vehicle has not been added"));
  }

  state = main;
}

void ChangeParkingLocation(String RegNumber, String location) {
  //Check if vehicle parking location can be changed, if it can, then changes it before returning to the main state
  CheckParkingLocation(location);
  bool found = false;
  for (int i = 0; i < StructSize; i++) {
    if (VehicleInfo[i].RegNumber == RegNumber) {
      found = true;
      if (VehicleInfo[i].ParkingLocation == location) {
        Serial.println(F("ERROR: Cannot be modified as same parking location"));
        break;
      } else if (VehicleInfo[i].PaymentStatus == "NPD") {
        Serial.println(F("ERROR: Cannot be modified as non payment status"));
        break;
      } else {
        VehicleInfo[i].ParkingLocation = location;
        Serial.println(F("DEBUG: Location changed"));
        display = true;
        break;
      }
    }
  }
  if (found == false) {
    Serial.println(F("ERROR: This vehicle has not been added"));
  }
  state = main;
}

void RemovingVehicle(String RegNumber) {
  bool found = false;
  for (int i = 0; i < StructSize; i++) {
    //Finding the vehicle to be removed
    if (VehicleInfo[i].RegNumber == RegNumber) {
      found = true;
      //Checks if it can be removed
      if (VehicleInfo[i].PaymentStatus == "NPD") {
        Serial.println(F("ERROR: Cannot be modified as non payment status"));
        break;
      } else {
        for (int j = i; j < (StructSize - 1); j++) {
          //Removes vehicle by shuffling all the vehicles after it, up by 1
          //Therefore the vehicle no longer exists within the struct
          VehicleInfo[j].RegNumber = VehicleInfo[j + 1].RegNumber;
          VehicleInfo[j].VehicleType = VehicleInfo[j + 1].VehicleType;
          VehicleInfo[j].PaymentStatus = VehicleInfo[j + 1].PaymentStatus;
          VehicleInfo[j].ParkingLocation = VehicleInfo[j + 1].ParkingLocation;
          VehicleInfo[j].EnterParkingTime = VehicleInfo[j + 1].EnterParkingTime;
          VehicleInfo[j].PaidParkingTime = VehicleInfo[j + 1].PaidParkingTime;
        }
        count = count - 1;
      }
    }
  }
  if (found == false) {
    Serial.println(F("ERROR: This vehicle does not exist"));
  }
  //Checks to make sure it is not part of the Changing Payment function
  if (changePay == false) {
    state = main;
  }
}

void displayVehicleInfo(int position) {
  //Clearing screen of previously displayed information
  //Include boolean variable to check if vehicle has been displayed
  if (display == true) {
    lcd.clear();
    //Creating arrows on screen
    arrows(position);
    lcd.setCursor(1, 0);
    //lcd.print("REGNUMB");
    lcd.print(VehicleInfo[position].RegNumber);
    lcd.setCursor(1, 1);
    lcd.print(VehicleInfo[position].VehicleType);
    lcd.setCursor(3, 1);
    lcd.print(VehicleInfo[position].PaymentStatus);
    //Sets the backlight of lcd according to the payment status
    if (VehicleInfo[position].PaymentStatus == "NPD") {
      lcd.setBacklight(3);
    } else {
      lcd.setBacklight(2);
    }
    lcd.setCursor(7, 1);
    lcd.print(VehicleInfo[position].EnterParkingTime);
    lcd.setCursor(12, 1);
    lcd.print(VehicleInfo[position].PaidParkingTime);
    lcd.setCursor(9, 0);
    //Implementing SCROLL extension
    //Checks if the length of the parking location is greater than 7 characters
    if (VehicleInfo[position].ParkingLocation.length() > 7) {
      //Assigns the parking location to a string variable called "loc"
      String loc = VehicleInfo[position].ParkingLocation;
      //Splits the string into a character array of the parking location to be added
      loc.toCharArray(locationScroll, loc.length() + 1);
      //Checks to see if any buttons have been pressed or any information has been inputted through the serial monitor
      while ((buttons() == false) && (Serial.available() == 0)) {
        //Create a variable that will change the position of the characters that will be displayed
        static int Position = 0;
        currentTime = millis();
        //Scrolling at 2 characters per second
        if (currentTime - lastTime >= 500) {
          //This is where the parking location starts
          lcd.setCursor(9, 0);
          //Prints out each letter of the parking location to the screen
          for (int i = 0; i < 8; ++i) {
            lcd.print(locationScroll[(i + Position) % strlen(locationScroll)]);
          }
          //Increment position to create "scrolling" effect
          ++Position;
          //Change the previous time to the current time to maintain scrolling at 2 characters per second
          lastTime = currentTime;
        }
      }
    } else {
      //If the parking location is 7 characters or less, then it just prints the parking location to the lcd
      //No need for scrolling effect as it fits onto the lcd screen
      lcd.print(VehicleInfo[position].ParkingLocation);
    }
  }
  display = false;
}

void arrows(int position) {
  //Create custom up arrow using a byte array
  byte upArrow[8] = {
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000
  };

  //Create custom down arrow using a byte array
  byte downArrow[8] = {
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111,
    0b01110,
    0b00100
  };

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  if ((VehicleBeforeIndex(position) == false) && (VehicleAfterIndex(position) == false)) {
    //If there is only 1 vehicle in the array then there should not be any arrows displayed
    lcd.setCursor(0, 0);
  } else if (VehicleBeforeIndex(position) == false) {
    //If there is vehicle is the first in the struct, then there must only be a down arrow
    lcd.setCursor(0, 1);
    lcd.write((uint8_t)1);
  } else if (VehicleAfterIndex(position) == false) {
    //If there is vehicle is the last in the struct, then there must only be an up arrow
    lcd.setCursor(0, 0);
    lcd.write((uint8_t)0);
  } else {
    //Otherwise, both arrows should be displayed as the vehicle is neither first or last in the struct
    lcd.setCursor(0, 0);
    lcd.write((uint8_t)0);
    lcd.setCursor(0, 1);
    lcd.write((uint8_t)1);
  }
}

bool VehicleAfterIndex(int index) {
  //Checks if there is a vehicle after the vehicle in the position specified by index
  if ((VehicleInfo[index + 1].RegNumber != "") && (index < StructSize - 1)) {
    return true;
  } else {
    return false;
  }
}

bool VehicleBeforeIndex(int index) {
  //Checks if there is a vehicle before the vehicle in the position specified by index
  if ((index > 0) && (VehicleInfo[index - 1].RegNumber != "")) {
    return true;
  } else {
    return false;
  }
}

bool buttons() {
  uint8_t buttons = lcd.readButtons();

  if (buttons & BUTTON_DOWN) {
    //Checks if there it a vehicle after the current vehicle
    //If there is, the next vehicle is displayed
    if (VehicleAfterIndex(displayedVehicle)) {
      displayedVehicle++;
      display = true;
      displayVehicleInfo(displayedVehicle);
    }
    return true;
  }

  if (buttons & BUTTON_UP) {
    //Checks if there it a vehicle before the current vehicle
    //If there is, the previous vehicle is displayed
    if (VehicleBeforeIndex(displayedVehicle)) {
      displayedVehicle--;
      display = true;
      displayVehicleInfo(displayedVehicle);
    }
    return true;
  }

  if (buttons & BUTTON_SELECT) {
    //Checks if the select button has been pressed for more than 1 second
    //If it has, it goes to the SELECT state
    //Otherwise, it starts counting the time
    if (selectPressed && (millis() > selectTime + 1000)) {
      state = SELECT;
    } else if (!selectPressed) {
      selectPressed = true;
      selectTime = millis();
    }
    return true;
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(5);  //Set backlight to purple
  currentTime = millis();
  lastTime = millis();
}

void loop() {
  currentTime = millis();
  switch (state) {
    case sync:
    //Create sync state
      {
        //Prints Q to the serial monitor every second
        while ((Serial.available() == 0) && (currentTime >= (lastTime + 1000))) {
          Serial.print("Q");
          lastTime = currentTime;
        }
        //Checks what has been inputted into the serial monitor
        if (Serial.available() > 0) {
          char input = Serial.read();
          if (input == 'X') {
            if (Serial.available() > 0) {
              Serial.print(F("ERROR: wrong format"));
              Serial.readString();
            }
            Serial.println("UDCHARS,FREERAM,SCROLL");
            lcd.setBacklight(7);  //Set backlight to white
            state = main;
          } else {
            if (Serial.available() > 0) {
              Serial.readString();
            }
            Serial.print(F("ERROR: wrong format"));
          }
        }
        break;
      }
    case main:
      {
        //Checks what has been inputted into the serial monitor
        if (Serial.available() > 0) {
          //Takes the input as a string
          String input = Serial.readString();

          //If input starts with A and is in correct format, then the vehicle is added to the struct
          if ((input[0] == 'A') && (input[1] == '-') && (input[9] == '-') && (input[11] == '-')) {
            CheckRegNumber(input.substring(2, 9));
            CheckVehicleType(input[10]);
            if ((CheckParkingLocation(input.substring(12))) == true) {
              AddToList(input.substring(2, 9), input[10], input.substring(12));
              display = true;
            }
            //displayVehicleInfo(count - 1);

          //If input starts with S and is in correct format, then vehicle payment status can be changed
          } else if ((input[0] == 'S') && (input[1] == '-') && (input[9] == '-')) {
            ChangePaymentStatus(input.substring(2, 9), input.substring(10));

          //If input starts with T and is in correct format, then vehicle type can be changed          
          } else if ((input[0] == 'T') && (input[1] == '-') && (input[9] == '-')) {
            ChangeVehicleType(input.substring(2, 9), input[10]);

            //If input starts with L and is in correct format, then vehicle parking location can be changed
          } else if ((input[0] == 'L') && (input[1] == '-') && (input[9] == '-')) {
            ChangeParkingLocation(input.substring(2, 9), input.substring(10));

            //If input starts with R and is in correct format, then vehicle can be removed
          } else if ((input[0] == 'R') && (input[1] == '-')) {
            RemovingVehicle(input.substring(2, 9));

            //Anything else is an error
          } else {
            if (Serial.available() > 0) {
              Serial.readString();
            }
            Serial.println(F("ERROR: Not correct input"));
          }
        }
        //check buttons
        buttons();

        //display vehicle
        displayVehicleInfo(displayedVehicle);
        break;
      }
    case SELECT:
      {
        if (!selectDisplayed) {
          //If the select button has been pressed, then the student ID number is displayed along with the amount of free ram displayed 
          lcd.clear();
          lcd.setBacklight(5);
          lcd.setCursor(0, 0);
          lcd.print("F319748");
          lcd.setCursor(0, 1);
          lcd.print("FREE RAM:");
          lcd.print(freeMemory());
          selectDisplayed = true;
        }
        uint8_t buttons = lcd.readButtons();
        if (!(buttons & BUTTON_SELECT)) {
          state = main;
          selectDisplayed = false;
          selectPressed = false;
          //If there is nothing in the struct, then then the display return to being white
          if (VehicleInfo[0].RegNumber == "") {
            lcd.clear();
            lcd.setBacklight(7); //Set backlight to white
          } else {
            display = true;
            displayVehicleInfo(displayedVehicle);
          }
        }
      }
      break;
  }
}