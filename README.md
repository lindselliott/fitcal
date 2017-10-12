# fitcal
_A CSV to ICS converter for raw exported Fitbit data._

The purpose of this converter is to have a place where you can turn your exported Fitbit data and imported into whatever calendar you use on an everyday basis. Sometimes it's nice to keep things all in one place. 

## How to Run: 
To Run the converter just hop in the fitcal directory and use the path to your exported Fitbit data (.csv) as your first and only argument. 

```
python fitcal.py fitbit_export_file.csv
```

You can export your Fitbit date [here](https://www.fitbit.com/premium/export)

## Output:
A .ics file will be exported in the same directory you are running the converter. You can use this file to import into your calendar and have all your data in one place.  

## References: 
ICS Standards: [RFC 2445](https://tools.ietf.org/html/rfc2445#section-4.6.3)
