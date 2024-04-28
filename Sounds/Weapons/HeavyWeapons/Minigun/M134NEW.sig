AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 4
   name "FrequencyShift"
   tl -270.5 28.667
   children {
    6
   }
   value 0.5
  }
  IOPItemInputClass {
   id 5
   name "VehicleFire"
   tl -270.5 -47.333
   children {
    3
   }
  }
  IOPItemInputClass {
   id 7
   name "VehicleFireReleased"
   tl -269.5 99.667
   children {
    8
   }
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 3
   name "VehicleFire"
   tl 129.5 -47.333
   input 5
  }
  IOPItemOutputClass {
   id 6
   name "FrequencyShift"
   tl 128.5 27.667
   input 4
  }
  IOPItemOutputClass {
   id 8
   name "VehicleFireReleased"
   tl 127.5 100.667
   input 7
  }
 }
 compiled IOPCompiledClass {
  visited {
   261 262 133 6 5 134
  }
  ins {
   IOPCompiledIn {
    data {
     1 65538
    }
   }
   IOPCompiledIn {
    data {
     1 2
    }
   }
   IOPCompiledIn {
    data {
     1 131074
    }
   }
  }
  outs {
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
  }
  processed 6
  version 2
 }
}