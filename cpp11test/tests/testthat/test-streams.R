test_that("streams do not return a warning about 'Compiled code should not...'",
{
  expect_message(streams_cout())
  expect_equal(streams_cout(), streams_cout2())

  expect_error(streams_cerr())
  expect_error(streams_cerr2())
})
