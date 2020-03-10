from unittest.mock import patch
from unittest import TestCase
import sys
import unittest
import lis
from io import StringIO


def get_input(text):
    return input(text)

def answer():
    ans = get_input('enter yes or no')
    if ans == 'yes':
        return 'you entered yes'
    if ans == 'no':
        return 'you entered no'

class TestTest(TestCase):
    def setUp(self):
        self.captor = StringIO()
        sys.stdout = self.captor

    def tearDown(self):
        sys.stdout = sys.__stdout__

    # get_input will return 'yes' during this test
    @patch('.get_input', return_value='yes')
    def test_answer_yes(self, input):
        self.assertEqual(answer(), 'you entered yes')

    @patch('yourmodule.get_input', return_value='no')
    def test_answer_no(self, input):
        self.assertEqual(answer(), 'you entered no')


if __name__ == "__main__":
    unittest.main()