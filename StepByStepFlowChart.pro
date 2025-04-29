# Add these lines to your .pro file:

# Handle translations
TRANSLATIONS += \
    i18n/StepByStepFlowChart_zh_CN.ts

# Update translations
CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += \
    resources.qrc

# The rest of your .pro file continues...
