module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Please change as you like..."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "More Settings"
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMPERATURE_SCALE",
        "label": "Do you like Kanji ?",
        "options": [
          { 
            "label": "Yes, I do", 
            "value": "YID" 
          },
          { 
            "label": "Of course, yes", 
            "value": "OCY" 
          }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];