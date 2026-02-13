# Copyright (c) CCP 2012

"""
Wrapper module to access blue exposed things from EVE's Localization DLL.
"""
from collections import OrderedDict
import unittest
import blue
import eveLocalization as el
import EveLocalizationTest

def GetTestData():
    testData = {
        "en-us":
                {
                0: (u"This is a string with no tags.",
                    None,
                    None
                    ),
                1: (u"This is a {thing} with no parameters.",
                        {
                        'foo': u'bar',
                        },
                    OrderedDict([
                        (u"{thing}",
                             {
                             'conditionalValues': [],
                             'variableType': el.VARIABLE_TYPE.GENERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': 0,
                             'variableName': 'thing'
                         }),
                    ])
                    ),
                2: (u"This string tests four types of modification of the latin alphabet: "
                    "{string1}, {string2}, {string3}, and {string4}.",
                    None,
                    OrderedDict([
                        (u"{string1}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.CAPITALIZE,
                             "variableName": "string1"
                         }),
                        (u"{string2}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.UPPERCASE,
                             "variableName": "string2"
                         }),
                        (u"{string3}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.LOWERCASE,
                             "variableName": "string3"
                         }),
                        (u"{string4}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.TITLECASE,
                             "variableName": "string4"
                         }),
                    ])
                    ),
                3: (u'You have {[numeric]numSeconds} {[numeric]numSeconds -> "second", "seconds"} to comply.',
                    None,
                    OrderedDict([
                        (u'{[numeric]numSeconds}',
                             {
                             'conditionalValues': [],
                             'variableType': el.VARIABLE_TYPE.NUMERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': 0,
                             'variableName': 'numSeconds',
                             }),
                        (u'{[numeric]numSeconds -> "second", "seconds"}',
                             {
                             'conditionalValues': [u'second', u'seconds'],
                             'variableType': el.VARIABLE_TYPE.NUMERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': el.TOKEN_FLAG.CONDITIONAL | el.TOKEN_FLAG.QUANTITY,
                             'variableName': 'numSeconds',
                             }),
                    ])
                    ),
                4: (u'This is a {thing} reusing a {thing} parameter.',
                    None,
                        {
                        u"{thing}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.GENERIC,
                                'kwargs': {},
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'thing',
                                },
                        }
                    ),
                5: (u'Datetime formatting test: {[datetime]time, timeFormat=short}',
                    None,
                        {
                        u"{[datetime]time, timeFormat=short}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.DATETIME,
                                'kwargs': { "format": u"%Y.%m.%d %H:%M" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                6: (u'Datetime formatting test: {[datetime]time, timeFormat=long}',
                    None,
                        {
                        u"{[datetime]time, timeFormat=long}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.DATETIME,
                                'kwargs': { "format": u"%Y.%m.%d %H:%M:%S" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                7: (u'This is {[messageid]msg} within a message',
                    None,
                        {
                        u'{[messageid]msg}':
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.MESSAGE,
                                'kwargs': {},
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'msg'
                            }
                    }
                    ),
                8: (u'a message',
                    None,
                    None
                    ),
                9: (u'Datetime formatting test: {[formattedtime]time, format="%A, %B %d %Y, %I:%M:%S %p"}',
                    None,
                        {
                        u'{[formattedtime]time, format="%A, %B %d %Y, %I:%M:%S %p"}':
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.FORMATTEDTIME,
                                'kwargs': { "format": u"%A, %B %d %Y, %I:%M:%S %p" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                10: (u'Numeric formatting test: {[numeric]number}',
                     None,
                         {
                         u'{[numeric]number}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': {},
                                 'propertyName': None,
                                 'args': 0,
                                 'variableName': 'number',
                                 },
                         }
                    ),
                11: (u'Numeric formatting test: {[numeric]number, decimalPlaces=0}',
                     None,
                         {
                         u'{[numeric]number, decimalPlaces=0}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': { "decimalPlaces": int(0), },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.DECIMALPLACES,
                                 'variableName': 'number',
                                 }
                     }
                    ),
                12: (u'Numeric formatting test: {[numeric]number, useGrouping}',
                     None,
                         {
                         u'{[numeric]number, useGrouping}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': {},
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.USEGROUPING,
                                 'variableName': 'number',
                                 }
                     }
                    ),
                13: (u'Numeric formatting test: {[numeric]number, decimalPlaces=2}',
                     None,
                         {
                         u'{[numeric]number, decimalPlaces=2}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': { "decimalPlaces": 2 },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.DECIMALPLACES,
                                 'variableName': 'number'
                             }
                     }
                    ),
                14: (u'This is a linkinfo link: {[generic]linktext, linkinfo=linkdata}',
                     None,
                         {
                         u'{[generic]linktext, linkinfo=linkdata}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.GENERIC,
                                 'kwargs': { "linkinfo": "linkdata" },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.LINKINFO,
                                 'variableName': 'linktext'
                             }
                     }
                    ),
                },
        "de":
                {
                1: (),
                },
        }
    return testData


testData = GetTestData()


class LocalizationUnittests(unittest.TestCase):
    """
    Make sure that the python exposure actually works as
    intended and prevent regressions from happening.
    """

    def setUp(self):
        el.LoadMessageData("en-us", testData["en-us"])

    def tearDown(self):
        el.UnloadMessageData("en-us")

    def testInvalidMessageDataTuple(self):
        """
        We don't know where this malformed data would come from, 
        but here is the defect: http://defects/issue.asp?ISID=76880
        """
        testData = {
            "ru":
                    {
                    0: (),
                    },
            }
        self.assertRaises(TypeError, el.LoadMessageData, 'ru', testData['ru'])

    def testCollator(self):
        """See if collator works."""
        col = el.Collator()
        col.locale = "en-us"
        self.assertEqual(col.locale, "en-us", "Collator has the wrong locale %s"%(col.locale,))
        res = col.Compare(u"a", u"b")
        self.assertEqual(res, -1, "a is not smaller than b")
        res = col.Compare(u"a", u"a")
        self.assertEqual(res, 0, "a is not a")
        res = col.Compare(u"b", u"a")
        self.assertEqual(res, 1, "b is not greater than a")

    def testLoadingData(self):
        """
        See if we can successfully store and retrieve data
        """
        data = el.GetMessageDataByID(1, "en-us")
        self.assertTrue(data == testData["en-us"][1], "message data storage does not return the same data for ID 1.")
        data = el.GetMessageDataByID(2, "en-us")
        self.assertTrue(data == testData["en-us"][2], "message data storage does not return the same data for ID 2.")

    def testLoadingFlatbufferData(self):
        """
        Test loading message data from a flatbuffer byte array in memory
        """
        
        # Load the flatbuffer data from memory
        el.LoadMessageDataFromFlatbuffer("en-us", EveLocalizationTest.GenerateValidFlatbuffersTestData())
        
        # Test Message 0: Simple message with no tags
        self.assertTrue(el.IsValidMessageID(0, "en-us"), "Message ID 0 should exist")
        result = el.GetMessageByID(0, "en-us")
        expectedResult = u"This is a simple test message."
        self.assertEqual(result, expectedResult, 
                        "Message 0 text mismatch: %s != %s" % (result, expectedResult))
        
        # Test Message 1: Message with a generic token
        self.assertTrue(el.IsValidMessageID(1, "en-us"), "Message ID 1 should exist")
        result = el.GetMessageByID(1, "en-us", name="World")
        expectedResult = u"Hello World!"
        self.assertEqual(result, expectedResult,
                        "Message 1 text mismatch: %s != %s" % (result, expectedResult))
        
        # Test Message 2: Message with metadata
        self.assertTrue(el.IsValidMessageID(2, "en-us"), "Message ID 2 should exist")
        result = el.GetMessageByID(2, "en-us")
        expectedResult = u"Test message with metadata"
        self.assertEqual(result, expectedResult,
                        "Message 2 text mismatch: %s != %s" % (result, expectedResult))
        metadata = el.GetMetaDataByID(2, "context", "en-us")
        expectedMetadata = u"test_context"
        self.assertEqual(metadata, expectedMetadata,
                        "Message 2 metadata mismatch: %s != %s" % (metadata, expectedMetadata))
        
        # Test Message 3: Message with numeric token and kwargs
        self.assertTrue(el.IsValidMessageID(3, "en-us"), "Message ID 3 should exist")
        result = el.GetMessageByID(3, "en-us", count=5)
        expectedResult = u"You have 5 items."
        self.assertEqual(result, expectedResult,
                        "Message 3 text mismatch: %s != %s" % (result, expectedResult))

    def testLoadingFlatbufferDataFailures(self):
        """
        Test error handling for LoadMessageDataFromFlatbuffer
        """
        # Empty buffer
        empty_buffer = bytearray([])
        self.assertRaises(ValueError, el.LoadMessageDataFromFlatbuffer,
                         "en-us", empty_buffer)
        
        # Invalid flatbuffer data
        self.assertRaises(ValueError, el.LoadMessageDataFromFlatbuffer,
                         "en-us", EveLocalizationTest.GenerateInvalidFlatbuffersTestData())
        
        # Valid flatbuffer structure but no messages
        self.assertRaises(ValueError, el.LoadMessageDataFromFlatbuffer,
                         "en-us", EveLocalizationTest.GenerateEmptyFlatbuffersTestData())

    def testParser(self):
        """
        Ensuring that our parser is providing all required functionality. Things that are not covered here rely upon
        undefined behaviour and thus may change and break in the future.
        """
        # no tags
        result = el.GetMessageByID(0, "en-us")
        expectedResult = u"This is a string with no tags."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        # a basic tag
        result = el.GetMessageByID(1, "en-us", thing="basic tag")
        expectedResult = u"This is a basic tag with no parameters."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # tag reuse
        result = el.GetMessageByID(4, "en-us", thing="basic tag")
        expectedResult = u"This is a basic tag reusing a basic tag parameter."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        # testing generic formatting options
        #result = GetMessageByID(2, "en-us", string1="capitalization of strings",
        #                        string2="uppercase", string3="LOWERCASE", string4="title case")
        #expectedResult = u"This string tests four types of modification of the latin alphabet: " \
        #                  "Capitalization of strings, UPPERCASE, lowercase, and Title Case."
        #self.assertTrue(result == expectedResult,
        #                "Result did not match input: %s != %s" % (result, expectedResult))

        # formatted datetime, short (ID: 5) and long (ID: 6), both for all three cases (bluetime, tuple, float)
        tupleTimeVal = (2011, 2, 24, 11, 55, 00, 3, 55, 0)
        blueTimeVal  = blue.os.GetTimeFromParts(*tupleTimeVal[:7])
        floatTimeVal = 1.0 * 1298548500
        result = el.GetMessageByID(5, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(5, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(5, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))

        result = el.GetMessageByID(6, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(6, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(6, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        # formatted time - accepts a format string directly
        result = el.GetMessageByID(9, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(9, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(9, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # recursive messages
        result = el.GetMessageByID(7, "en-us", msg=8)
        expectedResult = u"This is a message within a message"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))

        # Numeric Quantities
        result = el.GetMessageByID(10, "en-us", number=9999999)
        expectedResult = u"Numeric formatting test: 9999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(10, "en-us", number=9999999L)
        expectedResult = u"Numeric formatting test: 9999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(10, "en-us", number=9999999.12)
        expectedResult = u"Numeric formatting test: 9999999.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(11, "en-us", number=9999999.12)
        expectedResult = u"Numeric formatting test: 9999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(12, "en-us", number=9999999.123)
        expectedResult = u"Numeric formatting test: 9,999,999.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=9999999.123)
        expectedResult = u"Numeric formatting test: 9999999.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=9999999.1)
        expectedResult = u"Numeric formatting test: 9999999.10"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=-9999999.1)
        expectedResult = u"Numeric formatting test: -9999999.10"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        # Numeric conditional statements
        result = el.GetMessageByID(3, "en-us", numSeconds=20)
        expectedResult = u"You have 20 seconds to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(3, "en-us", numSeconds=1)
        expectedResult = u"You have 1 second to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(3, "en-us", numSeconds=1.7)
        expectedResult = u"You have 1.70 seconds to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # Link creation mechanism
        result = el.GetMessageByID(14, "en-us", linktext="Click Here!", linkdata=("linkinfo", 100, 200, 300, 400, 500))
        expectedResult = u"This is a linkinfo link: <a href=linkinfo:100//200//300//400//500>Click Here!</a>"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))


    def testNumericFormatter(self):
        """
        This is invoking the standalone method for number formatting
        """
        result = el.FormatNumeric(999999, "en-us", useGrouping=False, decimalPlaces=0)
        expectedResult = u"999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = u"999,999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True)
        expectedResult = u"999,999.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True, decimalPlaces=1)
        expectedResult = u"999,999.1"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = u"999,999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-999999.123, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = u"-999,999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-999999.123, "en-us", useGrouping=True, decimalPlaces=2)
        expectedResult = u"-999,999.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(0.0, "en-us")
        expectedResult = u"0.00" # decimalPlaces defaults to 2!
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=2)
        expectedResult = u"-09.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9000.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=5)
        expectedResult = u"-09,000.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=5)
        expectedResult = u"-00,009.12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))


    def testWrapPoints(self):
        """
        Word wrapping tests...
        """
        a = el.WrapPointList(u"Hello World", "en-us")
        result = a.GetLinebreakPoints()
        expectedResult = [6,]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        b = el.WrapPointList(u"The quick brown fox jumped over the lazy dog", "en-us")
        result = b.GetLinebreakPoints()
        expectedResult = [4, 10, 16, 20, 27, 32, 36, 41]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        c = el.WrapPointList(u"EVE\u59D0\u59B9\u4F1A\u7684\u4E3B\u8981\u804C\u8D23\u662F\u5411\u90A3\u4E9B\u906D\u9047\u6218\u4E89\u3001\u9965\u8352\u751A\u81F3\u8FF7\u5931\u592A\u7A7A\u7684\u4EBA\u63D0\u4F9B\u4EBA\u9053\u4E3B\u4E49\u63F4\u52A9\u3002\u7136\u800C\uFF0CEVE\u59D0\u59B9\u6D4E\u4F1A\u7684\u5B58\u5728\u5EFA\u7ACB\u4E8E\u4E0E\u79D1\u5B66\u56E0\u7D20\u76F8\u5173\u7684\u5F3A\u5927\u5B97\u6559\u4FE1\u4EF0\u4E4B\u4E0A\u3002\u4ED6\u4EEC\u76F8\u4FE1EVE\u4E4B\u95E8\u662F\u5929\u5802\u7684\u5165\u53E3\u2014\u2014\u795E\u5728\u95E8\u7684\u53E6\u5916\u4E00\u8FB9\u3002\u4ED6\u4EEC\u4E0D\u5149\u5E2E\u52A9\u90A3\u4E9B\u9700\u8981\u5E2E\u52A9\u7684\u4EBA\uFF0C\u8FD8\u5FD9\u4E8E\u8FDB\u884C\u5173\u4E8EEVE\u4E4B\u95E8\u7684\u79D1\u5B66\u8BD5\u9A8C\uFF0C\u5E0C\u671B\u80FD\u591F\u66F4\u6DF1\u5165\u5730\u4E86\u89E3\u90A3\u91CC\u8574\u6DB5\u7684\u529B\u91CF\u3002", "zh")
        result = c.GetLinebreakPoints()
        expectedResult = [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22,
                          23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 40, 42,
                          46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
                          62, 63, 64, 65, 66, 67, 68, 69, 70, 73, 74, 75, 76, 80, 81, 82, 83,
                          84, 85, 86, 90, 91, 92, 93, 94, 95, 96, 99, 100, 101, 102, 103,
                          104, 105, 106, 107, 108, 109, 110, 111, 114, 115, 116, 117, 118,
                          119, 120, 124, 125, 126, 127, 128, 129, 132, 133, 134, 135, 136, 137,
                          138, 139, 140, 141, 142, 143, 144, 145, 146, 147]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        d = el.WrapPointList(u"1, 4,   5, 6. 1. 2. 3.", "en-us")
        result = d.GetLinebreakPoints()
        expectedResult = [3, 8, 11, 14, 17, 20]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

    def testWrapPointsForRTLLanguage(self):
        """
        Input is an RTL language (arabic) string. Should not contain word wrap points, especially not at position 0.
        """
        s = u"\u0633\u064a\u0645\u062a\u0634"
        d = el.WrapPointList(s, "en-us")
        self.assertFalse(d.GetLinebreakPoints())

    @unittest.skipUnless(sys.platform.startswith("darwin"), "requires macOS")
    def testWrapPointsForTextWithInvalidUnicodeCharactersDontCrashOnMacOS(self):
        """ See https://ccpgames.atlassian.net/browse/PLAT-9947
        """
        # the sample text was extracted from the input that caused the crash
        text = u' \ud83d Spieler, die Lust auf Mining, Produktion oder PVE haben'
        with self.assertRaises(SystemError):
            # construction now raises a SystemError instead of silently creating a nullptr
            d = el.WrapPointList(text, "en-us")
            # this line should no longer get executed, but it is what caused the crash initially
            d.GetLinebreakPoints()

if __name__ == '__main__':
    unittest.main()
